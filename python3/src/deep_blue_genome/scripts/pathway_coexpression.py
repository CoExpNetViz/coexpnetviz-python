# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

'''
Generate graphs about coexpression between pathway genes
'''

import pandas as pd
import plumbum as pb
import numpy as np
import click
import matplotlib.pyplot as plt
import deep_blue_genome.core.context as ctx
from deep_blue_genome.core.reader.various import read_genes_file
from deep_blue_genome.core.util import is_data_file
from deep_blue_genome.util.plumbum import list_files
from deep_blue_genome.main import group, main
from deep_blue_genome.core.correlation import get_correlations, pearson_r,\
    get_correlations_sample
from deep_blue_genome.util.itertools import window
import logging
from deep_blue_genome.util.pandas import df_has_null
from matplotlib.cbook import boxplot_stats
from contextlib import contextmanager

_logger = logging.getLogger('deep_blue_genome.scripts.script')

class Context(ctx.CacheMixin, ctx.DatabaseMixin, ctx.TemporaryFilesMixin, ctx.OutputMixin):
    pass

@click.command()
@ctx.cli_options(Context) #TODO we still have version on this? Add to cli_options if not
@click.pass_obj
def script(main_config, **kwargs):
    kwargs['main_config'] = main_config
    context = Context(**kwargs)
    db = context.database
    
    # All pathway files
    paths = list(list_files(map(pb.local.path, ('/mnt/data/doc/work/prod_data/ARABIDOBSIS/pathways', '/mnt/data/doc/work/prod_data/rice/gois')), is_data_file))
    
    # Limit to the leafs of pathways (their name indicates the hierarchy)
    paths.sort()
    paths = [p1 for p1, p2 in window(paths) if not p2.startswith(p1)]
    
    # Load contents into data frame
    _logger.debug('load pathway files')
    def load_pathway(i, path):
        pathway = read_genes_file(path)
        pathway = pathway.to_frame('gene')
        pathway['group_id'] = i
        pathway['group_file'] = str(path)
        return pathway
    pathways = pd.concat((load_pathway(i, pathway) for i, pathway in enumerate(paths)), ignore_index=True)
    
    # Load Gene in gene group files
    # XXX put reading gene groups from paths in a func in database which encapsulates the trickery
    _logger.debug('load genes from DB')
    genes = db.get_genes_by_name(pathways['gene'], map_=True)
    pathways = pathways.drop('gene', axis=1).join(genes, how='inner')
    pathways.drop_duplicates(['group_id', 'gene'], inplace=True) # XXX dups might be caused by line above doing a join on index? hmm, not really. Or maybe becuase there may have been dups in the input already? Or because different gene names mapped to the same Gene, that's possible with synonyms. But we don't have synonyms yet atm. We map_, 2 left genes could map to the same right gene; should be possible. That might also answer the question we had in db exp mat data reading
    
    # Add group_size column
    _logger.debug('add size')
    sizes = pathways.groupby('group_id').size()
    sizes.name = 'group_size'
    pathways = pathways.join(sizes, on='group_id', how='inner')
    
    # Eliminate outliers: Brute way, limit to ad-hoc picked percentiles. Should use outside of 1.5 IQR range or more; why that instead of percentile range though?
    _logger.debug('eliminate outliers')
    cutoffs = sizes.quantile([.05, .95])
    lower_cutoff, upper_cutoff = cutoffs.iloc[0], cutoffs.iloc[1]
    pathways = pathways[(pathways.group_size >= lower_cutoff) & (pathways.group_size <= upper_cutoff)]
    
    # Load matching expression matrices
    _logger.debug('list relevent expression matrices')
    result = db.get_gene_collections_by_genes(pathways[['group_id', 'gene']], min_genes_present=8, expression_matrices=True)
    result = pd.merge(pathways.drop('gene', axis=1).drop_duplicates(), result.expression_matrices, on='group_id', how='inner')
    
    # Remove pathways without matching expression matrices
    pathways = pathways[pathways.group_id.isin(result.group_id)]
    
    #
    def finalise(corrs, group_id, expression_matrix, is_sampled): 
        np.fill_diagonal(corrs.values, np.nan) # ignore comparisons to self  # TODO does this work for pathway genes? no, not on diag because rows bigger or yes yes, should work.
        
        # Create graph
        
        # Keep stats only
        corrs = corrs.values.flatten()
        corrs = corrs[~np.isnan(corrs)]
        corrs = np.abs(corrs) # anti-corr or corr is all the same to us
        stats = boxplot_stats(corrs)[0]
        stats['group_id'] = group_id
        stats['expression_matrix'] = pb.local.path(expression_matrix.path).stem
        stats['is_sampled'] = is_sampled
        return stats
        
    _logger.debug('loop')
    all_corrs = []
    for expression_matrix, rest1 in result.groupby('expression_matrix'):
        _logger.info(expression_matrix.path)
        expression_matrix_ = db.get_expression_matrix_data(expression_matrix)
        

        for group_id, group in rest1.groupby('group_id'):
            # Random corrs
            random_corrs = pd.DataFrame(get_correlations_sample(expression_matrix_, pearson_r, len(group))) # note: we actually want random cols=rows as it best represents what we look for on baits
            all_corrs.append(finalise(random_corrs, group_id, expression_matrix, True))
#             assert not df_has_null(random_corrs)  # XXX low variance has not been removed from all data sets yet apparently
            
            # Bait corrs
            corrs = get_correlations(expression_matrix_[expression_matrix_.index.isin(group.gene)], group.gene, pearson_r)        
#             assert not df_has_null(corrs)
            all_corrs.append(finalise(corrs, group_id, expression_matrix, False))
#             break
#         break

    _logger.debug('merging results')
    result = pd.DataFrame(all_corrs)
    quartile = 'q3'
    result = result[['group_id', 'expression_matrix', quartile, 'is_sampled']]
    
    sampled = result[result.is_sampled].drop('is_sampled', axis=1)
    sampled.rename(columns={quartile: 'sampled'}, inplace=True)
    
    not_sampled = result[~result.is_sampled].drop('is_sampled', axis=1)
    not_sampled.rename(columns={quartile: 'pathway'}, inplace=True)
    
    result = pd.merge(sampled, not_sampled, on=['group_id', 'expression_matrix'], how='inner')
    
    _logger.debug('generating figures')
    
    quartiles = ['pathway', 'sampled']
    plot_args = dict(figsize=(20, 11), xlim=(0, 1), alpha=0.5, colormap='Dark2')
    
    # sample medians
    plt.clf()
    result[quartiles].plot.hist(
        title='histogram of {} coexpression (of pathway/sampled genes) across all pathways and matrices'.format(quartile), 
        **plot_args)
    plt.savefig('hist_{}_all.png'.format(quartile))
    
    # medians per matrix: how much does median vary within an expression matrix as pathway changes
    plt.figure(figsize=plot_args['figsize'])
    plt.suptitle('histogram of {} coexpression per matrix of pathways'.format(quartile))
    groups = result.groupby('expression_matrix')[quartiles]
#     layout = [np.ceil(np.sqrt(len(groups)))]*2
    layout = [4,5]
    for i, (expression_matrix, data) in enumerate(groups):
        #XXX pandas v18 will have a working plot.hist(by=), currently it's broken so we use this loop. Could extract a func, throw it in pandas, for now
        plt.subplot(*(layout + [i+1]))
        patches = plt.hist([data['pathway'].values, data['sampled'].values], label=quartiles)[2]
        plt.title(expression_matrix)
        plt.xlim(plot_args['xlim'])
    plt.gcf().legend(patches, quartiles)
        
    plt.savefig('hist_{}s_within_matrix_across_pathways.png'.format(quartile))
    
    # std per matrix: how much does median vary within a pathway as exp mat changes
    plt.clf()
    result.groupby('group_id')[quartiles].std().plot.hist(title='histogram of std of {} coexpression within pathway as matrix varies'.format(quartile), **plot_args)
    plt.savefig('hist_std_within_pathway_across_matrices.png')
    
    _logger.debug('done')
    
    
group.add_command(script)
main(['script', '--cache-dir', '.'])
    
    