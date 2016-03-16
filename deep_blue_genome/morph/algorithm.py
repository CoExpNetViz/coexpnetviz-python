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
from chicken_turtle_util.itertools import window
from more_itertools.more import chunked

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
from deep_blue_genome.core.correlation import pearson_r, get_correlations

_logger = logging.getLogger('deep_blue_genome.morph') 

# XXX could perhaps squeeze out some more vectorisation

def _normalise(ranking):
    return (ranking - ranking.values.mean()) / ranking.values.std()
 
def _finalise(ranking, baits):
    '''
    Finalise ranking, see _rank_genes implementation
    
    baits : pd.Series([Gene])
    '''
    non_baits = ~ranking.index.isin(baits)
    ranking = ranking.loc[non_baits]
    if not ranking.empty:
        ranking = ranking / len(baits)
        ranking = _normalise(ranking)
    return ranking
    
def _get_auc(indices):
    return indices[indices < 1000].sum() / (1000 * len(indices))

_ReturnTuple = namedtuple('_ReturnTuple', ['ranking', 'ausr'])

# TODO need think about mapped genes on exp mat: duplicates may arise e.g. gene1 mapping to (mapped1, mapped2) and gene2 mapping to mapped2. Unless off course we can assume about mappings that no 2 genes map to the same mapped gene. First thing is to check whether that's true for MSU-RAP mapping.

# Performance notes:
# - Runtime of each (bait group, exp-mat, clustering) combination appears similar to C++ version, we just have a lot more clustering/exp-mat combinations to check now.
# - partial copy of ranking (df.loc[] = other.loc[]) was slower than fully copying
# - using df.values in normalise() had no effect
# - pd.Series(df.values.sum()) slowed performance by 10%
# XXX working entirely with numpy in here and sub-funcs, and avoid copying the numpy arrays should lead to a performance gain. Am curious how close that'd get to C++. Is there potential for Cython too?
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
    # XXX could speed up by switching to np. And with np might speed up by avoiding copies
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
            tmp_ranking = pre_ranking.copy()
    
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
    
    min_genes_present = 8
    _logger.info('For each bait group, use only expression matrices and clusterings which have at least {} baits in common.'.format(min_genes_present))
    _logger.info('For each expression matrix and clustering combination, temporarily reduce the current bait group to the baits both have in common. The excluded baits are also excluded from the AUSR calculation.')
    
    # fetch list of relevant clusterings and expression matricess from DB
    # XXX could query expmat, clust in one go, filtering by their common baits only. Or on the other hand, could all that contain at least 1 gene and filter here, if you want to mention as excluded in the logs; or to simplify the query since it might not make much performance difference to do it here instead; there just aren't many matrices and clusterings
    db = context.database
    result = db.get_gene_collections_by_genes(bait_groups, min_genes_present=min_genes_present, expression_matrices=True, clusterings=True)
    df = pd.merge(result.expression_matrices, result.clusterings, on=['group_id', 'gene'], how='inner')
    
    # Hardcoded combinations
    # TODO is is_enzyme a full clustering or does it contain only half of everything? E.g. just one clustering? It's not that hard to add the missing cluster, is it? Why should we or shouldn't we?
    exp_mats = '''
        E-GEOD-14275.expression_matrix  
        E-GEOD-25073.expression_matrix  
        E-GEOD-31077.expression_matrix  
        E-GEOD-5167.expression_matrix
        E-GEOD-8216.expression_matrix
        E-MEXP-2267.expression_matrix
        E-GEOD-35984.expression_matrix  
        E-GEOD-39298.expression_matrix
        RiceGenomeDataSet.expression_matrix
    '''.split()
    exp_mats = ['/mnt/data/doc/work/prod_data/rice/data_sets/' + x for x in exp_mats]
    
    clusterings = '''
        E-GEOD-14275_click.clustering          
        E-GEOD-14275_ppi_mcl_matisse.clustering
        E-GEOD-25073_click.clustering          
        E-GEOD-25073_ppi_mcl_matisse.clustering
        E-GEOD-31077_click.clustering          
        E-GEOD-31077_ppi_mcl_matisse.clustering
        E-GEOD-5167_click.clustering
        E-GEOD-5167_ppi_mcl_matisse.clustering
        E-GEOD-8216_click.clustering
        E-GEOD-8216_ppi_mcl_matisse.clustering
        E-MEXP-2267_click.clustering
        E-MEXP-2267_ppi_mcl_matisse.clustering
        E-GEOD-35984_click.clustering            
        E-GEOD-35984_ppi_mcl_matisse.clustering
        E-GEOD-39298_click.clustering            
        E-GEOD-39298_ppi_mcl_matisse.clustering
        RiceGenomeDataSet_click.clustering
        RiceGenomeDataSet_ppi_mcl_matisse.clustering
        is_enzyme.clustering
    '''.split()
    clusterings = ['/mnt/data/doc/work/prod_data/rice/clusterings/' + x for x in clusterings]
    clustering = clusterings[-1]
    clusterings = clusterings[0:-1]
    
    acceptable_combinations = (
        [(exp_mat, clustering) for exp_mat in exp_mats] +
        [(exp_mat, clustering) for exp_mat, clusterings_ in zip(exp_mats, chunked(clusterings, 2)) for clustering in clusterings_] +
        [('/mnt/data/doc/work/prod_data/ARABIDOBSIS/data_sets/' + exp_mat, '/mnt/data/doc/work/prod_data/ARABIDOBSIS/cluster_solution/' + clustering)
            for exp_mat, clustering
            in chunked('''
                ds1Data.txt                 DS1_click.txt
                ds1Data.txt                 ds1_ppi_matisse_0.4.txt
                Seed_GH_DataSet.txt   Seed_GH_CLICK_ClusteringSol.txt
                Seed_GH_DataSet.txt   Seed_GH_IsEnzymeClusteringSol.txt
                Seed_GH_DataSet.txt   Seed_GH_ppi_matisse_0.4.txt
                SeedsDataSet.txt Seeds_CLICK_ClusteringSol.txt
                SeedsDataSet.txt SeedsDataSet_Metabolic_Network_ClusteringSol.txt
                SeedsDataSet.txt SeedsDataSet_PPI_Network_ClusteringSol.txt
                Seed_GH_2_DataSet.txt    Seed_GH_2_ppi_matisse_0.4.txt
                Seed_GH_2_DataSet.txt    Seed_GH_2_CLICK_ClusteringSol.txt
                Seed_GH_2_DataSet.txt    Seed_GH_2_IsEnzymeClusteringSol.txt
                TissuesData.txt    Tissues_IsEnzymeClusteringSol.txt
                TissuesData.txt    Tissues_Metabolic_Network_ClusteringSol.txt
                TissuesData.txt    Tissues_PPI_Network_ClusteringSol.txt
                TissuesData.txt    Tissues_CLICK_ClusteringSol.txt
                SeedlingsDataSet.txt    Seedlings_IsEnzymeClusteringSol.txt
                SeedlingsDataSet.txt    Seedlings_Metabolic_Network_ClusteringSol.txt
                SeedlingsDataSet.txt    Seedlings_PPI_Network_ClusteringSol.txt
                SeedlingsDataSet.txt    Seedlings_CLICK_ClusteringSol.txt
                Root_DataSet.txt    Root_IsEnzymeClusteringSol.txt
                Root_DataSet.txt    Root_CLICK_ClusteringSol.txt
                Root_DataSet.txt    Root_MCL_PPI_reduced_extended_solution_0.4.txt
                Pollen_Boavida_DataSet.txt    Pollen_boavida_CLICK_ClusteringSol.txt
                Pollen_Boavida_DataSet.txt    Pollen_boavida_IsEnzymeClusteringSol.txt
                Pollen_Boavida_DataSet.txt    Pollen_boavida_matisseimprove_0.4.txt
                Pollen_Lin_DataSet.txt    Pollen_lin_CLICK_ClusteringSol.txt
                Pollen_Lin_DataSet.txt    Pollen_lin_IsEnzymeClusteringSol.txt
                Pollen_Lin_DataSet.txt    Pollen_lin_matisseimprove_0.4.txt
                Lateral_Root_DataSet.txt    Lateral_CLICK_ClusteringSol.txt
                Lateral_Root_DataSet.txt    Lateral_IsEnzymeClusteringSol.txt
                Lateral_Root_DataSet.txt    Lateral_matisseimprove_0.4.txt
            '''.split(), 2)
        ]
    )
    
    # Main alg
    rankings = []
    for group_id, rest in df.groupby('group_id'):
        bait_group = bait_groups[bait_groups['group_id']==group_id]['gene']
        for expression_matrix, rest2 in rest.groupby('expression_matrix'):
            expression_matrix_ = db.get_expression_matrix_data(expression_matrix)
            
            # Calculate correlations
            correlations = get_correlations(expression_matrix_, bait_group, pearson_r)
            
            for clustering, baits_present in rest2.groupby('clustering')['gene']:
                if (expression_matrix.path, clustering.path) not in acceptable_combinations:
                    continue
                
                if len(baits_present) >= min_genes_present:
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
                    db.session.commit()  # Commit every now and then to prevent database from running out of resources (e.g. table lock space)
    
    return pd.DataFrame(rankings, columns=('bait_group_id', 'expression_matrix', 'clustering', 'baits_present', 'baits_missing', 'ausr', 'ranking'))
    
    
    