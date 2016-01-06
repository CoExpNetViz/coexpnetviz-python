# Copyright (C) 2015, 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Blue Genome.
# 
# Deep Blue Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Blue Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
from collections import namedtuple

'''
The MORPH algorithm
'''

# The algorithm is explained in Diels T.'s thesis p11, 18 and 19

import pandas as pd
import numpy as np
import plumbum as pb
import logging
from deep_blue_genome.core.reader.various import read_expression_matrix_file,\
    read_clustering_file
from deep_blue_genome.core.metrics import pearson_r

_logger = logging.getLogger('deep_blue_genome.morph') 

# XXX could perhaps squeeze out some more vectorisation


# XXX would be nice to have an interface to wrap around e.g. pearson_r to say 'these indices is the subset, restore index afterwards'. Share this func modified with coexpnetviz
def _get_correlations(expression_matrix, subset, correlation_method):
    mask = expression_matrix.index.isin(subset)
    correlations = correlation_method(expression_matrix.values, np.flatnonzero(mask))
    correlations = pd.DataFrame(correlations, index=expression_matrix.index, columns=expression_matrix.index[mask])
    return correlations

def _normalise(ranking):
    return (ranking - ranking.mean()) / ranking.std()
 
def _finalise(ranking, baits):
    '''
    Finalise ranking, see _rank_genes implementation
    
    baits : pd.Series([Gene])
    '''
    non_baits = ~ranking.index.isin(baits)
    ranking = ranking.loc[non_baits]
    ranking = ranking / len(baits)
    return _normalise(ranking)
    
def _get_auc(indices):
    return indices[indices < 1000].sum() / (1000 * len(indices))

_ReturnTuple = namedtuple('_ReturnTuple', ['ranking', 'ausr'])

# TODO need think about mapped genes on exp mat: duplicates may arise e.g. gene1 mapping to (mapped1, mapped2) and gene2 mapping to mapped2. Unless off course we can assume about mappings that no 2 genes map to the same mapped gene. First thing is to check whether that's true for MSU-RAP mapping.
def _rank_genes(correlations, clustering, baits):
    clustering = clustering.set_index('item')
    
    # Take only relevant bait columns
    correlations = correlations[baits]
    
    # Take only row genes present in clustering and add cluster_id
    correlations = correlations.join(clustering, how='inner')
    
    # XXX zip(groupby, groupby, ...) would make some of this code prettier
    # XXX what's the speed of the easiest to read possible implementation?
    # Calculate ranking
    pre_ranking = []
    ranking = []
    for _, corrs in correlations.groupby('cluster_id'): # XXX would apply() be faster?
        corrs = corrs.drop('cluster_id', axis=1)
        cluster_baits = corrs.columns.intersection(corrs.index).to_series()  # baits in cluster
        if not cluster_baits.empty: # only clusters with baits
            pre_ranking.append(corrs[cluster_baits].sum(axis=1))
            ranking.append(_finalise(pre_ranking[-1], cluster_baits))  # XXX could avoid copy, worth it to avoid copy? Let's avoid it later
    pre_ranking = pd.concat(pre_ranking)
    ranking = pd.concat(ranking)
    
    # For each bait, leave it out and calculate ranking based on pre_ranking, and check its position in the ranking
    tmp_ranking = pre_ranking.copy()
    indices = []
    for _, corrs in correlations.groupby('cluster_id'):
        corrs = corrs.drop('cluster_id', axis=1)
        cluster_baits = corrs.columns.intersection(corrs.index).to_series()  # baits in cluster
        if not cluster_baits.empty:
            for bait in cluster_baits:
                tmp_ranking.loc[corrs.index] = pre_ranking.loc[corrs.index] - corrs[bait]
                _finalise(tmp_ranking.loc[corrs.index], cluster_baits.drop(bait))
                tmp_ranking.sort_values(inplace=True)
                indices.append(tmp_ranking.index.get_loc(bait))
#             tmp_ranking = pre_ranking.copy() # XXX potential to speed up by not copying, see line below
            tmp_ranking.loc[corrs.index] = pre_ranking.loc[corrs.index]
#             print(len(tmp_ranking))
#             print(len(pre_ranking))
#             assert tmp_ranking.sort_values().equals(pre_ranking.sort_values())
    
    # Apply area under the curve to indices to get the AUSR
    ausr = _get_auc(pd.Series(indices))
    
    return _ReturnTuple(ranking=ranking, ausr=ausr) 

def morph(context, bait_groups, top_k):
    '''
    TODO
    
    TODO
    
    Parameters
    ----------
    bait_groups : pd.DataFrame(columns=[group_id : int, gene : Gene])
        list of gene collections to which non-bait genes are compared
    top_k : int
        K best candidate genes to output in ranking
        
    Returns
    -------
    pandas.DataFrame({'group_id' : int, 'ranking' : Ranking})
        Best Ranking for each group. Groups for which a Ranking could not be
        made are omitted.
    '''
    
    _logger.info('For each bait group, use only expression matrices and clusterings which have at least 5 baits in common.')
    _logger.info('For each expression matrix and clustering combination, temporarily reduce the current bait group to the baits both have in common. The excluded baits are also excluded from the AUSR calculation.')
    
    # fetch list of relevant clusterings and expression matricess from DB
    # XXX could query expmat, clust in one go, filtering by their common baits only. Or on the other hand, could all that contain at least 1 gene and filter here, if you want to mention as excluded in the logs; or to simplify the query since it might not make much performance difference to do it here instead; there just aren't many matrices and clusterings
    db = context.database
    result = db.get_gene_collections_by_genes(bait_groups, min_genes_present=5, expression_matrices=True, clusterings=True)
    df = pd.merge(result.expression_matrices, result.clusterings, on=['group_id', 'gene'], how='inner')
    
    # Main alg
    rankings = []
    for group_id, rest in df.groupby('group_id'):
        bait_group = bait_groups[bait_groups['group_id']==group_id]['gene']
        for expression_matrix, rest2 in rest.groupby('expression_matrix'):
            expression_matrix_ = read_expression_matrix_file(pb.local.path(expression_matrix.path)).data # TODO keep pb.local.path or load from db as such? Probably should load as pb straight away for max convenience
            
            # Swap gene names for actual genes
            # XXX Loading genes on the index of an exp-mat is going to be a fairly common operation, want a func for it?
            matrix_genes = db.get_genes_by_name(expression_matrix_.index.to_series(), map_=True)
            expression_matrix_ = expression_matrix_.reindex(matrix_genes.index)
            expression_matrix_.index = matrix_genes
            assert not expression_matrix_.index.has_duplicates  # currently assuming this never happens
            
            # Calculate correlations
            correlations = _get_correlations(expression_matrix_, bait_group, pearson_r)
            
            for clustering, baits_present in rest2.groupby('clustering')['gene']:
                if len(baits_present) >= 5:
                    clustering_ = read_clustering_file(pb.local.path(clustering.path), name_index=1)
                    clustering_ = clustering_.drop('item', axis=1).join(db.get_genes_by_name(clustering_['item'], map_=True))
                    _logger.info(
                        'Calculating ranking for bait group {}:\n- expression matrix: {}\n- clustering: {}\n- {} baits present in both matrix and clustering, {} baits missing'
                        .format(group_id, expression_matrix.path, clustering.path, len(baits_present), len(bait_group) - len(baits_present))
                    )
                    result = _rank_genes(correlations, clustering_, baits_present)
                    _logger.info('Resulting ranking has AUSR={}'.format(result.ausr))
                    baits_missing = bait_group[~bait_group.isin(baits_present)]
                    rankings.append((group_id, expression_matrix, clustering, baits_present, baits_missing, result.ausr, result.ranking.iloc[0:top_k].copy()))
    
    return pd.DataFrame(rankings, columns=('bait_group_id', 'expression_matrix', 'clustering', 'baits_present', 'baits_missing', 'ausr', 'ranking'))
    
    
    