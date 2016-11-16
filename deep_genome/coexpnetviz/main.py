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

from deep_genome.coexpnetviz import __version__, write_cytoscape, create_network, _interpret, _parse, ExpressionMatrix
from deep_genome.core import Context as CoreContext, correlation, clean, parse, interpret
from deep_genome.core.database import Credentials, import_
from chicken_turtle_util import click as click_, series as series_, path as path_, logging as logging_
from chicken_turtle_util.configuration import ConfigurationLoader
from pkg_resources import resource_string
from pathlib import Path
import pandas as pd
import click
import matplotlib
import logging
import plumbum as pb
import numpy as np
import shutil
import matplotlib.pyplot as plt

_logger = logging.getLogger(__name__)

class Context(object):
    
    def __init__(self, configuration_path):
        loader = ConfigurationLoader('deep_genome.coexpnetviz', 'deep_genome_coexpnetviz', 'coexpnetviz')
        configuration = loader.load(configuration_path)
        
        core_credentials = Credentials(
            configuration['core']['database_host'],
            configuration['core']['database_name'],
            configuration['core']['database_user'],
            configuration['core']['database_password']
        )
        
        self._core = CoreContext(
            core_credentials
        )
        
    def __getattr__(self, attr):
        return getattr(self._core, attr)

_file_parameter_type = click.Path(exists=True, dir_okay=False)

@click.command(context_settings={'help_option_names': ['-h', '--help']})
@click_.option('--configuration', type=click.Path(exists=True), required=False, help='Configuration file or directory.')
@click_.option('-b', '--baits', type=_file_parameter_type, help='File listing bait genes')
@click_.option('-e', '--expression-matrix', 'expression_matrices', type=_file_parameter_type, multiple=True, help='Gene expression matrix file')
@click_.option('-f', '--gene-families', type=_file_parameter_type, required=False, help='Gene families file')
@click_.option(
    '--correlation-function', 
    default='pearson', 
    type=click.Choice(['pearson', 'mutual-information']), 
    help='Correlation function to use for measuring gene coexpression'
)
@click_.option(
    '--percentile-ranks',
    default=(5.0, 95.0),
    help=
    'Which percentile rank to use to determine the lower and upper cut-off. For '
    'each expression matrix, a sample of genes is drawn and the correlations '
    'between them is calculated. From this sample distribution of correlations '
    'the L-th percentile is used as a cut-off to determine whether 2 genes in the '
    'matrix are co-expressed or not.'
)
@click.version_option(version=__version__)
def main(baits, expression_matrices, gene_families, correlation_function, percentile_ranks, configuration):
    '''
    Comparative Co-Expression Network Construction and Visualization
    (CoExpNetViz)
    '''
    logging_.configure('coexpnetviz.log')
    logging.getLogger().setLevel(logging.INFO)
    logging.getLogger('deep_genome.coexpnetviz').setLevel(logging.DEBUG)
    
    # Log versions
    _logger.info('deep_genome.coexpnetviz version: {}'.format(__version__))
    _logger.debug('pip freeze:\n{}'.format(pb.local['pip']('freeze')))
    
    # Convert click to pathlib paths
    baits = Path(baits)
    expression_matrices = [Path(matrix) for matrix in expression_matrices]
    if gene_families:
        gene_families = Path(gene_families)
    
    # Copy input files
    _copy_input_files(baits, expression_matrices, gene_families)
        
    # Log other input
    _logger.debug('correlation function: {}'.format(correlation_function))
    _logger.debug('percentile ranks: {}'.format('{}, {}'.format(*percentile_ranks)))
    
    # Init matplotlib
    if not 'DISPLAY' in pb.local.env:
        matplotlib.use('Agg')  # use this backend when no X server
    
    # Context
    context = Context(Path(str(configuration)))
    
    # correlation_function
    correlation_function_name = correlation_function
    if correlation_function == 'pearson':
        correlation_function = correlation.pearson_df
    elif correlation_function == 'mutual-information':
        correlation_function = correlation.mutual_information_df
    else:
        assert False  # programming error
        
    #
    with context.database.scoped_session() as session:
        # Read baits
        baits = pd.Series(clean.plain_text(baits.open()).read().split())
        baits = series_.split(session.get_genes_by_name(baits))
        
        # Read gene_families
        if gene_families:
            with open(str(gene_families)) as f:
                gene_families = _interpret.gene_families(session, _parse.gene_families(clean.plain_text(f)))
        else:
            gene_families = pd.DataFrame(columns=('family', 'gene'))
        
        # Read expression_matrices
        matrices = []
        for matrix in expression_matrices:
            matrix = Path(str(matrix))
            with matrix.open() as f:
                matrices.append(ExpressionMatrix(
                    matrix.name,  # Note: if the name is not unique across matrices, it's up to the user to rename them to be unique
                    interpret.expression_matrix(session, parse.expression_matrix(clean.plain_text(f)))
                ))
        expression_matrices = matrices
        
        # Run algorithm
        network = create_network(baits, expression_matrices, gene_families, correlation_function, percentile_ranks)
 
        # Write network to cytoscape files
        write_cytoscape(network, 'network')
        
        # Write intermediate files 
        for expression_matrix, sample, correlation_matrix in zip(expression_matrices, network.samples, network.correlation_matrices):
            name = expression_matrix.name
            
            sample.index = sample.index.map(lambda gene: gene.name)
            sample.columns = sample.columns.map(lambda gene: gene.name)
            sample.to_csv(name + '.sample_matrix.txt', sep='\t', na_rep=str(np.nan))
            
            correlation_matrix.index = correlation_matrix.index.map(lambda gene: gene.name)
            correlation_matrix.columns = correlation_matrix.columns.map(lambda gene: gene.name)
            correlation_matrix.to_csv(name + '.correlation_matrix.txt', sep='\t', na_rep=str(np.nan))
            
        percentiles = pd.DataFrame(network.percentiles, columns=('lower', 'upper'))
        percentiles.insert(0, 'expression_matrix', expression_matrices)
        percentiles['expression_matrix'] = percentiles['expression_matrix'].apply(lambda matrix: matrix.name)
        percentiles.to_csv('percentiles.txt', sep='\t', na_rep=str(np.nan), index=False)
        
        network.significant_correlations[['bait', 'gene']] = network.significant_correlations[['bait', 'gene']].applymap(lambda gene: gene.name)
        network.significant_correlations.to_csv('significant_correlations.txt', sep='\t', na_rep=str(np.nan), index=False)
        
        # Write readme
        path_.write(Path('README.txt'), resource_string(__name__, 'data/README.txt').decode())
        
        # Write histograms of samples
        for matrix, sample, percentiles in zip(expression_matrices, network.samples, network.percentiles):
            # Flatten sample matrix
            sample_size = len(sample.index)
            sample = sample.values.copy()
            np.fill_diagonal(sample, np.nan)
            sample = sample[~np.isnan(sample)].ravel()
            
            # Write histogram
            line_style = dict(color='r', linewidth=2)
            plt.clf()
            pd.Series(sample).plot.hist(bins=60)
            plt.title('Correlations between sample of\n{} genes in {}'.format(sample_size, matrix.name))
            plt.xlabel(correlation_function_name)
            plt.ylabel('frequency')
            plt.axvline(percentiles[0], **line_style)
            plt.axvline(percentiles[1], **line_style)
            plt.savefig('{}.sample_histogram.png'.format(matrix.name))
            
            # Write cdf
            plt.clf()
            pd.Series(sample).plot.hist(bins=60, cumulative=True, normed=True)
            plt.title('Cumulative distribution of correlations\nbetween sample of {} genes in {}'.format(sample_size, matrix.name))
            plt.xlabel(correlation_function_name)
            plt.ylabel('Cumulative probability, i.e. $P(corr \leq x)$')
            plt.axhline(percentile_ranks[0]/100.0, **line_style)
            plt.axhline(percentile_ranks[1]/100.0, **line_style)
            plt.savefig('{}.sample_cdf.png'.format(matrix.name))

def _copy_input_files(baits, expression_matrices, gene_families):
    input_directory = Path('input')
    input_directory.mkdir()
    
    baits_directory = input_directory / 'baits'
    baits_directory.mkdir()
    shutil.copy(str(baits), str(baits_directory / baits.name))
    
    matrices_directory = input_directory / 'expression_matrices'
    matrices_directory.mkdir()
    for expression_matrix in expression_matrices:
        shutil.copy(str(expression_matrix), str(matrices_directory / expression_matrix.name))
        
    if gene_families:
        families_directory = input_directory / 'gene_families'
        families_directory.mkdir()
        shutil.copy(str(gene_families), str(families_directory / gene_families.name))
        
def _write_sample_histogram(sample, sample_size, file_name, correlation_function, cumulative):
    plt.clf()
    pd.Series(sample).plot.hist(bins=30, cumulative=cumulative)
    plt.title('Correlations between sample of {} genes in expression matrix'.format(sample_size))
    plt.xlabel(correlation_function.__name__)
    plt.ylabel('{}frequency'.format('cumulative ' if cumulative else ''))
    plt.savefig(file_name)
    