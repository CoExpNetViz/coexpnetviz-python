# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Genome.
# 
# Deep Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Genome.  If not, see <http://www.gnu.org/licenses/>.

from deep_genome.coexpnetviz import __version__
from deep_genome.core import AlgorithmContext
# from deep_genome.coexpnetviz.correlationmethod import CorrelationMethod
# from deep_genome.coexpnetviz.cytoscapewriter import CytoscapeWriter
# from deep_genome.coexpnetviz.algorithm import coexpnetviz
# from deep_genome.core.reader.various import read_genes_file, read_expression_matrix_file, read_gene_families_file
# import sys
# import pandas as pd

Context = AlgorithmContext(__version__)

@Context.command()
def main():
    '''
    TODO description, maybe ref to project and doc links too
    '''
    
    # Parse CLI args
#     def percentile_rank_type(value):
#         value_ = float(value)
#         if value_ < 0 or value_ > 100:
#             raise argparse.ArgumentTypeError("{} is not a value between 0 and 100 (inclusive)".format(value))
#         return value_
#     
#     parser = argparse.ArgumentParser(description='Comparative Co-Expression Network Construction and Visualization (CoExpNetViz): Command line interface')
#     parser.add_argument(
#         '--baits-file', metavar='B', required=True,
#         help='Path to file listing the bait genes to use.'
#     )
#     parser.add_argument(
#         '--gene-families', metavar='F',
#         help='Path to file with gene families to use.'
#     )
#     parser.add_argument(
#         '-e', '--expression-matrices', metavar='M', required=True, nargs='+',
#         help='Paths to expression matrices to use.'
#     )
#     parser.add_argument(
#         '--correlation-method', metavar='S',
#         choices=CorrelationMethod.__members__.keys(), #TODO show choices in -h, maybe ConfigArgParse does this? Maybe it's a setting?
#         default=CorrelationMethod.pearson_r.name,
#         help='Correlation method to use for gene coexpression.'
#     )
#     parser.add_argument(
#         '--lower-percentile-rank', metavar='L',
#         default=5, type=percentile_rank_type,
#         help='Which percentile rank to use to determine the lower cut-off. For each expression matrix, a sample of genes is drawn and the correlations between them is calculated. From this sample distribution of correlations, the L-th percentile is used as a cut-off to determine whether 2 genes in the matrix are co-expressed or not.'
#     )
#     parser.add_argument(
#         '--upper-percentile-rank', metavar='U',
#         default=95, type=percentile_rank_type,
#         help='Which percentile rank to use to determine the upper cut-off. See --lower-percentile-rank for details.'
#     )
#     args = parser.parse_args(argv[1:])
# 
#     # Read files
#     baits = read_genes_file(args.baits_file)
#     expression_matrices = [read_expression_matrix_file(matrix) for matrix in args.expression_matrices] # TODO should remove non-varying rows in user-submitted exp mats (in any exp mats really, but we can trust our own prepped mats already have this step performed)
#     if args.gene_families:
#         gene_families = read_gene_families_file(args.gene_families)
#     else:
#         gene_families = pd.Series([], name='gene')
#         gene_families.index.name = 'family'
#     
#     # Pick metric
#     correlation_method = CorrelationMethod[args.correlation_method]
#     
#     # Run alg
#     network = coexpnetviz(baits, gene_families, expression_matrices, correlation_method, [args.lower_percentile_rank, args.upper_percentile_rank])
# 
#     # Write network to cytoscape files
#     CytoscapeWriter(network).write()

#TODO include in doc
'''
Terminology used:

- bait gene: One of the genes provided by the user to which target genes are compared in terms of co-expression
- target gene: any gene that's not a bait gene
- family node: a node containing targets of the same orthology family
'''