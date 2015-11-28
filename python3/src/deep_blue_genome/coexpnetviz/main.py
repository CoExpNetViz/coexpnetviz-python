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

from deep_blue_genome.core.reader.various import read_baits_file, read_expression_matrix_file,\
    read_gene_families_file
import sys
import argparse
import pandas as pd
import numpy as np
from deep_blue_genome.core.util import series_invert
from deep_blue_genome.coexpnetviz.similaritymetric import SimilarityMetric
from deep_blue_genome.coexpnetviz.network import Network
from deep_blue_genome.coexpnetviz.cytoscapewriter import CytoscapeWriter
import matplotlib.pyplot as plt

# TODO wouldn't it be interesting to show correlation between baits as well?

'''
CoExpNetViz

Terminology used:

- bait gene: One of the genes provided by the user to which target genes are compared in terms of co-expression
- target gene: any gene that's not a bait gene
- family node: a node containing targets of the same orthology family
'''

def determine_cutoffs(expression_matrix, similarity_metric):
    '''
    Get upper and lower correlation cutoffs for coexpnetviz
    
    Takes the 5th and 95th percentile of a sample similarity matrix of
    `expression_matrix`, returning these as the lower and upper cut-off
    respectively.
    
    Parameters
    ----------
    expression_matrix : Expressionmatrix
    similarity_metric : SimilarityMetric
    
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
    
    sample_size = 800
    data = expression_matrix.data.values
    sample = np.random.choice(len(data), sample_size)
    sample = data[sample]
    sample = similarity_metric(sample, np.arange(len(sample)))
    sample = sample.flatten()
    sample = sample[~np.isnan(sample)]
    
    # Also save a histogram
    plt.clf()
    pd.Series(sample).plot.hist(bins=30)
    plt.title('Correlations between sample of {} genes in exp-mat'.format(sample_size))
    plt.xlabel(similarity_metric.name)
    plt.savefig('{}.corr_sample_histogram.png'.format(expression_matrix.name))

    # Return result
    return np.percentile(sample, [5, 95])
    
def coexpnetviz(context, baits, gene_families, expression_matrices, similarity_metric):
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
    similarity_metric : SimilarityMetric
        
    Returns
    -------
    Network
        A network/graph of typed nodes, edges and partitions 
    '''
    inverted_gene_families = series_invert(gene_families)
    
    # Correlations
    # Note: for genes not part of any family we assume they are part of some family, just not one of the ones provided. (so some family nodes have None as family)
    correlations = []
    for exp_mat in expression_matrices:
        lower_bound, upper_bound = determine_cutoffs(exp_mat, similarity_metric)
        matrix = exp_mat.data
        
        # Baits present in matrix
        baits_mask = matrix.index.isin(baits)
        baits_ = matrix.index[baits_mask]
        
        # Correlation matrix
        corrs = similarity_metric(matrix.values, np.flatnonzero(baits_mask))
        corrs = pd.DataFrame(corrs, index=matrix.index, columns=baits_)
        corrs.to_csv(exp_mat.name + '.sim_mat.txt', sep='\t', na_rep=str(np.nan))
        
        # correlations
        corrs.columns = baits_
        corrs.drop(baits_, inplace=True)
        corrs = corrs[(corrs > lower_bound) & (corrs < upper_bound)]
        corrs.dropna(how='all', inplace=True)  # TODO not sure if aids performance
        corrs = corrs.join(inverted_gene_families)
        corrs.index.name = 'family_gene'
        corrs.reset_index(inplace=True)
        corrs = pd.melt(corrs, id_vars=['family_gene', 'family'], var_name='bait', value_name='correlation')
        corrs.dropna(subset=['correlation'], inplace=True)
        correlations.append(corrs)
    
    correlations = pd.concat(correlations)
    correlations = correlations.reindex(columns='family family_gene bait correlation'.split())
    
    # Return
    return Network(
        name='network',
        baits=baits,
        gene_families=gene_families,
        correlations=correlations
    )


# Why get the geneid? Some genes have multiple names, with mere strings they wouldn't match, after mapping them to their id, they would.

# We don't actually care what organism (or taxon rather) a gene is of. We just want all the matrices that have in it, the genes that we are looking for 


def main():
    main_(sys.argv)

def main_(argv):
    # Parse CLI args
    parser = argparse.ArgumentParser(description='Comparative Co-Expression Network Construction and Visualization (CoExpNetViz): Command line interface')
    parser.add_argument(
        '--baits-file', metavar='B', required=True,
        help='path to file listing the bait genes to use'
    )
    parser.add_argument(
        '--gene-families', metavar='F',
        help='path to file with gene families to use. If omitted, Plaza is used'
    )
    parser.add_argument(
        '-e', '--expression-matrices', metavar='M', required=True, nargs='+',
        help='path to expression matrix to use'
    )
    parser.add_argument(
        '--similarity-metric', metavar='S',
        choices=SimilarityMetric.__members__.keys(), #TODO show choices in -h, maybe ConfigArgParse does this? Maybe it's a setting?
        default=SimilarityMetric.pearson_r.name,
        help='similarity metric to use for gene coexpression'
    )
    args = parser.parse_args(argv[1:])

    # Read files
    baits = read_baits_file(args.baits_file)
    gene_families = read_gene_families_file(args.gene_families) if args.gene_families else None
    expression_matrices = [read_expression_matrix_file(matrix) for matrix in args.expression_matrices] # TODO should remove non-varying rows in user-submitted exp mats (in any exp mats really, but we can trust our own prepped mats already have this step performed)
    
    # Pick metric
    similarity_metric = SimilarityMetric[args.similarity_metric]
    
    # Run alg
    context = None # TODO
    network = coexpnetviz(context, baits, gene_families, expression_matrices, similarity_metric)

    # Write network to cytoscape files
    CytoscapeWriter(network).write()

if __name__ == '__main__':
    main()
    
    
    
