# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

import pandas as pd
import numpy as np
from chicken_turtle_util.pandas import series_invert
from deep_blue_genome.coexpnetviz.network import Network
import matplotlib.pyplot as plt
from deep_blue_genome.core.correlation import get_correlations_sample

def determine_cutoffs(expression_matrix, correlation_method, percentile_ranks):
    '''
    Get upper and lower correlation cutoffs for coexpnetviz
    
    Takes the 5th and 95th percentile of a sample similarity matrix of
    `expression_matrix`, returning these as the lower and upper cut-off
    respectively.
    
    Parameters
    ----------
    expression_matrix : Expressionmatrix
    correlation_method : CorrelationMethod
    
    Returns
    -------
    (lower, upper)
        Cut-offs
    '''
    # TODO we took a sample of the population of correlations, so take into
    # account statistics when drawing conclusions from it... In fact, that's how
    # we should determine our sample size, probably.
    #
    # Before that, take a step back and compare some form of significance vs using a percentile of a sample as cutoff
    
    sample = get_correlations_sample(expression_matrix, correlation_method)
    sample = sample.flatten()
    sample = sample[~np.isnan(sample)]
    
    # Also save a histogram
    plt.clf()
    pd.Series(sample).plot.hist(bins=30)
    plt.title('Correlations between sample of {} genes in exp-mat'.format(sample_size))
    plt.xlabel(correlation_method.name)
    plt.savefig('{}.corr_sample_histogram.png'.format(expression_matrix.name))#TODO no name attrib

    # Return result
    return np.percentile(sample, percentile_ranks)
    
def coexpnetviz(baits, gene_families, expression_matrices, correlation_method, percentile_ranks):
    '''
    Derive a CoExpNetViz Network.
    
    Bait node iff bait (even when no fam node correlates to it)
    
    Family nodes only contain the genes of the family that actually correlate to
    a bait. Families that don't correlate with any bait are omitted from the
    output.
    
    Partitions are the grouping of fam nodes by the subset of baits they correlate to.
    
    Parameters
    ----------
    baits : see `read_baits_file`'s return value
        genes to which non-bait genes are compared
    gene_families : see `read_gene_families_file`'s return value
        gene families of the genes in the expression matrices. This may be omitted if all baits are of the same species
    expression_matrices : list-like of ExpressionMatrix
        gene expression matrices containing some or all baits and other genes
    correlation_method : CorrelationMethod
        
    Returns
    -------
    Network
        A network/graph of typed nodes, edges and partitions 
    '''
    inverted_gene_families = series_invert(gene_families)
    
    # Correlations
    # Note: for genes not part of any family we assume they are part of some family, just not one of the ones provided. (so some family nodes have None as family)
    correlations = []
    baits_present = pd.Index([])
    for exp_mat in expression_matrices:
        lower_cutoff, upper_cutoff = determine_cutoffs(exp_mat, correlation_method, percentile_ranks)
        matrix = exp_mat.data
        
        # Baits present in matrix
        baits_mask = matrix.index.isin(baits)
        baits_ = matrix.index[baits_mask]
        baits_present = baits_present.union(baits_)
        
        # Correlation matrix
        corrs = correlation_method(matrix.values, np.flatnonzero(baits_mask))
        corrs = pd.DataFrame(corrs, index=matrix.index, columns=baits_)
        corrs.to_csv(exp_mat.name + '.sim_mat.txt', sep='\t', na_rep=str(np.nan))
        
        # correlations
        corrs.columns = baits_
        corrs.drop(baits_, inplace=True)
        corrs = corrs[(corrs < lower_cutoff) | (corrs > upper_cutoff)]
        corrs.dropna(how='all', inplace=True)  # TODO not sure if aids performance
        corrs = corrs.join(inverted_gene_families)
        corrs.index.name = 'family_gene'
        corrs.reset_index(inplace=True)
        corrs = pd.melt(corrs, id_vars=['family_gene', 'family'], var_name='bait', value_name='correlation')
        corrs.dropna(subset=['correlation'], inplace=True)
        correlations.append(corrs)
    
    correlations = pd.concat(correlations)
    correlations = correlations.reindex(columns='family family_gene bait correlation'.split())
    
    # All baits
    baits_missing = pd.Index(baits).difference(baits_present)
    print("Warning: baits missing: " + ' '.join(baits_missing.tolist())) # TODO logger instead
    
    # Return
    baits_present = pd.Series(baits_present, name=baits.name)
    baits_present.index.name = baits.index.name
    return Network(
        name='network',
        baits=baits_present,
        gene_families=gene_families,
        correlations=correlations
    )