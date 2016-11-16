# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

'''
Test the CLI: deep_genome.coexpnetviz.main

It's assumed that `create_network` and `write_cytoscape` is used under the hood.
We only test that arguments are passed correctly and user-friendly messages are
shown on invalid input. We do test in detail the files created in addition to
`write_cytoscape`.
'''

#TODO
'''
error reporting:

- when any case, clean error message, no stack trace
'''

#TODO also write corrs per family

from deep_genome.coexpnetviz.main import main
from chicken_turtle_util import path as path_, data_frame as df_, click as click_
from chicken_turtle_util.test import temp_dir_cwd, assert_text_equals, assert_text_contains
from pathlib import Path
from textwrap import dedent
import pandas as pd
import pytest

#TODO test mutual information as well (just a subset of the tests we currently have. Check sample, corr mat, percentiles on a single matrix, no gene fams)

@pytest.fixture(autouse=True)
def autouses(temp_dir_cwd, db):
    pass

@pytest.fixture()
def cli_test_args(test_conf_path):
    return ['--configuration', str(test_conf_path)]

@pytest.fixture
def matrix1():
    path = Path('matrix1')
    path_.write(path, dedent('''\
        \tcondition1\tcondition2\tcondition3
        gene1\t1\t2\t3
        gene2\t3\t2\t1
        gene3\t1\t2\t1
        gene4\t8\t3\t5'''
    ))
    return path

@pytest.fixture
def gene_families1():
    path = Path('gene_families1')
    path_.write(path, 'fam1\tgene3\tgene4')
    return path

@pytest.fixture
def matrix2():
    path = Path('matrix2')
    path_.write(path, dedent('''\
        \tcondition1\tcondition2\tcondition3\tcondition4
        geneB1\t1\t2\t3\t4
        geneB2\t4\t3\t2\t1'''
    ))
    return path

@pytest.fixture
def baits1():
    path = Path('baits1')
    path_.write(path, 'gene1 gene2')
    return path

@pytest.fixture
def baits_both():
    path = Path('baits_both')
    path_.write(path, 'gene1 gene2 geneB1')
    return path

def test_happy_days(cli_test_args, baits1, matrix1, capsys):
    '''
    When given correct baits and an expression matrix, run fine
    '''
    args = cli_test_args + ['-e', str(matrix1), '--baits', str(baits1)]
    click_.assert_runs(main, args)
    
    # Sample matrix file + default correlation function is pearson
    expected = pd.DataFrame(
        [
            [1, -1, 0, -0.59603956067926978],
            [-1, 1, 0, 0.59603956067926978],
            [0, 0, 1, -0.802955],
            [-0.59603956067926978, 0.59603956067926978, -0.802955, 1]
        ],
        index=['gene1', 'gene2', 'gene3', 'gene4'],
        columns=['gene1', 'gene2', 'gene3', 'gene4'],
    )
    actual = pd.read_table('matrix1.sample_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0,1}, all_close=True)
    
    # Correlation matrix file (+ pearson is used)
    expected = pd.DataFrame(
        [
            [1, -1],
            [-1, 1],
            [0, 0],
            [-0.59603956067926978, 0.59603956067926978]
        ],
        index=['gene1', 'gene2', 'gene3', 'gene4'],
        columns=['gene1', 'gene2'],
    )
    actual = pd.read_table('matrix1.correlation_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0}, all_close=True)
    
    # Percentiles file + default cutoffs are 5, 95
    expected = pd.DataFrame(
        [
            ['matrix1', -1, 0.59603956067926978]
        ],
        columns=['expression_matrix', 'lower', 'upper'],
    )
    actual = pd.read_table('percentiles.txt', index_col=None)
    df_.assert_equals(actual, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    
    # Significant correlations file
    expected = pd.DataFrame(
        [
            ['gene1', 'gene2', -1],
            ['gene2', 'gene4', 0.59603956067926978]
        ],
        columns=['bait', 'gene', 'correlation'],
    )
    actual = pd.read_table('significant_correlations.txt', index_col=None)
    df_.assert_equals(actual, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    
    # Cytoscape files present
    for file in ('network.edge.attr', 'network.node.attr', 'network.sif', 'coexpnetviz_style.xml'):
        assert Path(file).exists()
        
    # README.txt file present
    assert Path('README.txt').exists()
    
    # input directory
    path_.assert_equals(Path('input/baits') / baits1.name, baits1, mode=False)
    path_.assert_equals(Path('input/expression_matrices') / matrix1.name, matrix1, mode=False)
    assert not Path('input/gene_families').exists()
    
    # Log info sent to stderr, not debug
    _, stderr = capsys.readouterr()
    assert not 'D:' in stderr
    
    # Log debug sent to log file
    log = path_.read(Path('coexpnetviz.log'))
    
    # log version info and input not included in input dir
    assert_text_contains(log, 'pip freeze')
    assert_text_contains(log, 'correlation function: pearson')
    assert_text_contains(log, 'percentile ranks: 5.0, 95.0')
    
    # Sample graphs
    # Note: difficult to test automatically, check contents manually on release
    for file in ('matrix1.sample_histogram.png', 'matrix1.sample_cdf.png'):
        assert Path(file).exists()
    
def test_gene_families(cli_test_args, baits1, matrix1, gene_families1):
    '''
    When given gene families, use them
    '''
    args = cli_test_args + ['-e', str(matrix1), '--baits', str(baits1), '--gene-families', str(gene_families1)]
    click_.assert_runs(main, args)
        
    # gene families used
    assert 'fam1' in path_.read(Path('network.node.attr'))
    
    # gene fams include in input/ dir
    path_.assert_equals(Path('input/gene_families') / gene_families1.name, gene_families1, mode=False)
    
def test_correlation_function(cli_test_args, baits1, matrix1):
    '''
    When mutual information is requested, use it
    '''
    args = cli_test_args + ['-e', str(matrix1), '--baits', str(baits1), '--correlation-function', 'mutual-information']
    click_.assert_runs(main, args)
        
    # Used in sample
    expected = pd.DataFrame(
        [
            [1.0986122886681096, 1.0986122886681096, 0.63651416829481289, 1.0986122886681096],
            [1.0986122886681096, 1.0986122886681096, 0.63651416829481289, 1.0986122886681096],
            [0.63651416829481289, 0.63651416829481289, 0.63651416829481289, 0.63651416829481289],
            [1.0986122886681096, 1.0986122886681096, 0.63651416829481289, 1.0986122886681096]
        ],
        index=['gene1', 'gene2', 'gene3', 'gene4'],
        columns=['gene1', 'gene2', 'gene3', 'gene4'],
    )
    actual = pd.read_table('matrix1.sample_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0,1}, all_close=True)
    
    # Used in correlation matrix
    expected = pd.DataFrame(
        [
            [1.0986122886681096, 1.0986122886681096],
            [1.0986122886681096, 1.0986122886681096],
            [0.63651416829481289, 0.63651416829481289],
            [1.0986122886681096, 1.0986122886681096]
        ],
        index=['gene1', 'gene2', 'gene3', 'gene4'],
        columns=['gene1', 'gene2'],
    )
    actual = pd.read_table('matrix1.correlation_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0}, all_close=True)

def test_cutoffs(cli_test_args, baits1, matrix1):
    '''
    When given percentile ranks, use them
    '''
    args = cli_test_args + ['-e', str(matrix1), '--baits', str(baits1), '--percentile-ranks', '50', '50']
    click_.assert_runs(main, args)
        
    # Percentiles 50 50 are used
    expected = pd.DataFrame(
        [
            ['matrix1', -0.29802, -0.29802]
        ],
        columns=['expression_matrix', 'lower', 'upper'],
    )
    actual = pd.read_table('percentiles.txt', index_col=None)
    df_.assert_equals(actual, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    
def test_multiple_expression_matrices(cli_test_args, baits_both, matrix1, matrix2):
    '''
    When given multiple matrices (2), use both
    '''
    args = cli_test_args + ['-e', str(matrix1), '-e', str(matrix2), '--baits', str(baits_both)]
    click_.assert_runs(main, args)
    
    # Sample matrix files
    expected = pd.DataFrame(
        [
            [1, -1, 0, -0.59603956067926978],
            [-1, 1, 0, 0.59603956067926978],
            [0, 0, 1, -0.802955],
            [-0.59603956067926978, 0.59603956067926978, -0.802955, 1]
        ],
        index=['gene1', 'gene2', 'gene3', 'gene4'],
        columns=['gene1', 'gene2', 'gene3', 'gene4'],
    )
    actual = pd.read_table('matrix1.sample_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0,1}, all_close=True)
    
    expected = pd.DataFrame(
        [
            [1, -1],
            [-1, 1],
        ],
        index=['geneB1', 'geneB2'],
        columns=['geneB1', 'geneB2'],
    )
    actual = pd.read_table('matrix2.sample_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0,1}, all_close=True)
    
    # Correlation matrix files
    expected = pd.DataFrame(
        [
            [1, -1],
            [-1, 1],
            [0, 0],
            [-0.59603956067926978, 0.59603956067926978]
        ],
        index=['gene1', 'gene2', 'gene3', 'gene4'],
        columns=['gene1', 'gene2'],
    )
    actual = pd.read_table('matrix1.correlation_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0}, all_close=True)
    
    expected = pd.DataFrame(
        [
            [1],
            [-1],
        ],
        index=['geneB1', 'geneB2'],
        columns=['geneB1'],
    )
    actual = pd.read_table('matrix2.correlation_matrix.txt', index_col=0)
    df_.assert_equals(actual, expected, ignore_order={0}, all_close=True)
    
    # Percentiles file
    expected = pd.DataFrame(
        [
            ['matrix1', -1, 0.59603956067926978],
            ['matrix2', -1, -1]
        ],
        columns=['expression_matrix', 'lower', 'upper'],
    )
    actual = pd.read_table('percentiles.txt', index_col=None)
    df_.assert_equals(actual, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    
    # Significant correlations file
    expected = pd.DataFrame(
        [
            ['gene1', 'gene2', -1],
            ['gene2', 'gene4', 0.59603956067926978],
            ['geneB1', 'geneB2', -1]
        ],
        columns=['bait', 'gene', 'correlation'],
    )
    actual = pd.read_table('significant_correlations.txt', index_col=None)
    df_.assert_equals(actual, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    
    # input directory
    for matrix in (matrix1, matrix2):
        path_.assert_equals(Path('input/expression_matrices') / matrix.name, matrix, mode=False)
    
    # Sample graphs
    # Note: difficult to test automatically, check contents manually on release
    for matrix in (matrix1, matrix2):
        for file in ('{}.sample_histogram.png', '{}.sample_cdf.png'):
            assert Path(file.format(matrix.name)).exists()
    
# def manual():
#     # test the help message contains most things:
#     #
#     # - CLI synopsis
#     #     --lower-percentile-rank (default 5)
#     #     --upper-percentile-rank (default 95)
#     #     --baits (required)
#     #     --expression-matrices (required)
#     #     --correlation-function (default pearson)
#     #     --gene-families
#     # - nice --help message otherwise
#     print(CliRunner().invoke(main, ['--help']).output)
#     assert False
    