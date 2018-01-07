# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
#
# This file is part of CoExpNetViz.
#
# CoExpNetViz is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CoExpNetViz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with CoExpNetViz.  If not, see <http://www.gnu.org/licenses/>.

'''
Test coexpnetviz.create_network(...)
'''

from pytil import data_frame as df_, series as series_
from coexpnetviz import create_network, NodeType, RGB, ExpressionMatrix
from varbio import correlation
from itertools import product
from textwrap import dedent
from more_itertools import one
import pandas as pd
import numpy as np
import pytest

def ids_to_labels(labels, df, columns):
    df = df.copy()
    df[columns] = df[columns].apply(lambda column: column.map(labels), axis=1)
    return df

def combine_edges(df, column1, column2, labels=None):
    '''
    Combine edges and assert no self edges, no duplicates (taking into account synonymy)
    '''
    if labels is not None:
        df = ids_to_labels(labels, df, [column1, column2])
    else:
        df = df.copy()

    subset = df[[column1, column2]]
    df['combined'] = subset.apply(frozenset, axis=1) 
    del df[column1]
    del df[column2]

    # no self edges
    assert (df['combined'].apply(len) == 2).all(), df

    # no synonymous edges
    assert not df['combined'].duplicated().any(), df

    return df

def assert_network(
        input_, network, bait_nodes=None, family_nodes=None, gene_nodes=None,
        samples=None, correlation_matrices=None, percentiles=None, homology_edges=None,
        significant_correlations=None, correlation_edges=None, partitions=None
    ):
    '''
    Assert create_network's return value
    '''
    expected_bait_nodes = bait_nodes
    expected_family_nodes = family_nodes
    expected_gene_nodes = gene_nodes
    expected_homology_edges = homology_edges
    expected_significant_correlations = significant_correlations
    expected_correlation_edges = correlation_edges

    input_baits = set(input_['baits'].tolist())
    input_genes = set(sum((mat.data.index.tolist() for mat in input_['expression_matrices']), []))
    input_non_baits = input_genes - input_baits

    nodes = network.nodes
    bait_nodes = nodes[nodes['type'] == NodeType.bait]
    family_nodes = nodes[nodes['type'] == NodeType.family]
    gene_nodes = nodes[nodes['type'] == NodeType.gene]
    bait_node_ids = set(bait_nodes['id'].tolist())
    node_ids = set(nodes['id'].tolist())

    # samples
    assert len(network.correlation_matrices) == len(input_['expression_matrices'])
    for sample, exp_mat in zip(network.samples, input_['expression_matrices']):
        assert sample.columns.isin(exp_mat.data.index).all()
        assert sample.columns.is_monotonic_increasing  # sorted
        assert sample.index.isin(exp_mat.data.index).all()
        assert sample.index.is_monotonic_increasing  # sorted
        assert sample.shape[0] == sample.shape[1]
        assert (sample.dtypes == float).all()

    # correlation_matrices
    assert len(network.correlation_matrices) == len(input_['expression_matrices'])
    for corr_mat, exp_mat in zip(network.correlation_matrices, input_['expression_matrices']):
        assert corr_mat.columns.isin(input_baits).all()
        assert corr_mat.columns.is_monotonic_increasing  # sorted
        assert corr_mat.index.isin(exp_mat.data.index).all()
        assert corr_mat.index.is_monotonic_increasing  # sorted
        assert (corr_mat.dtypes == float).all()

    # percentiles
    assert len(network.percentiles) == len(input_['expression_matrices'])
    assert all(isinstance(lower, float) and isinstance(upper, float) for (lower, upper) in network.percentiles)

    # nodes columns
    assert (nodes.columns == ['id', 'label', 'type', 'genes', 'family', 'colour', 'partition_id']).all() 

    # node id
    assert nodes['id'].notnull().all()
    assert nodes['id'].dtype == int, nodes['id']
    assert not nodes['id'].duplicated().any()  # unique

    # node label
    assert nodes['label'].notnull().all()
    assert not nodes['label'].duplicated().any()  # unique
    assert (bait_nodes['label'] == bait_nodes['genes'].apply(one)).all()
    assert (family_nodes['label'] == family_nodes['family']).all()
    assert (gene_nodes['label'] == gene_nodes['genes'].apply(one)).all()
    labels = nodes.set_index('id')['label']

    # node colour
    assert nodes['colour'].notnull().all()
    assert nodes['colour'].apply(lambda x: isinstance(x, RGB)).all()  # RGB component values within bounds and are integer

    # node type
    assert nodes['type'].notnull().all()
    assert nodes['type'].apply(lambda x: isinstance(x, NodeType)).all()

    # node families
    def isnan(x):  # Note: cannot use np.isnan on non-float values such as None
        return x != x
    assert not nodes['family'].apply(isnan).any()  # No NaN, should use None instead
    assert gene_nodes['family'].isnull().all()
    assert family_nodes['family'].notnull().all()

    # node genes
    assert nodes['genes'].apply(lambda x: isinstance(x, frozenset)).all(), nodes['genes']
    assert (bait_nodes['genes'].apply(len) == 1).all()
    assert bait_nodes['genes'].apply(lambda x: one(x) in input_baits).all(), (bait_nodes['genes'], input_baits)
    assert family_nodes['genes'].apply(bool).all()  # all non-empty
    assert family_nodes['genes'].apply(lambda xs: all(x in input_non_baits for x in xs)).all()
    assert (gene_nodes['genes'].apply(len) == 1).all()
    assert gene_nodes['genes'].apply(lambda xs: all(x in input_non_baits for x in xs)).all()
    assert not series_.split(nodes['genes'].apply(list)).duplicated().any()  # no gene appears in 2 nodes

    # node partition_id
    assert nodes['partition_id'].notnull().all()
    assert nodes['partition_id'].dtype == int

    # each partition has exactly one colour
    tmp = nodes[['colour', 'partition_id']].drop_duplicates()
    assert not tmp['colour'].duplicated().any()
    assert not tmp['partition_id'].duplicated().any()

    # homology_edges
    assert (network.homology_edges.columns == ['bait_node1', 'bait_node2']).all()
    assert network.homology_edges.notnull().all().all(), network.homology_edges
    assert network.homology_edges.isin(bait_node_ids).all().all()
    homology_edges = combine_edges(network.homology_edges, 'bait_node1', 'bait_node2', labels)

    # significant_correlations
    assert network.significant_correlations.notnull().all().all()
    assert (network.significant_correlations.columns == ['bait', 'gene', 'correlation']).all()
    assert network.significant_correlations['correlation'].dtype == float
    assert network.significant_correlations['bait'].isin(input_baits).all()
    assert network.significant_correlations['gene'].isin(input_genes).all()
    significant_correlations = combine_edges(network.significant_correlations, 'bait', 'gene')

    # correlation_edges
    assert network.correlation_edges.notnull().all().all()
    assert (network.correlation_edges.columns == ['bait_node', 'node', 'max_correlation']).all()
    assert network.correlation_edges['max_correlation'].dtype == float
    assert network.correlation_edges['bait_node'].isin(bait_node_ids).all()
    assert network.correlation_edges['node'].isin(node_ids).all()
    tmp_node_ids = set(network.correlation_edges['node'].tolist())
    assert family_nodes['id'].isin(tmp_node_ids).all()  # each family correlates with a bait
    correlation_edges = combine_edges(network.correlation_edges, 'bait_node', 'node', labels)

    # optional checks
    if samples is not None:
        for actual, expected in zip(network.samples, samples):
            df_.assert_equals(actual, expected, ignore_order={0,1}, all_close=True)
    if correlation_matrices is not None:
        for actual, expected in zip(network.correlation_matrices, correlation_matrices):
            df_.assert_equals(actual, expected, ignore_order={0,1}, all_close=True)
    if percentiles is not None:
        actual = np.array(network.percentiles)
        expected = np.array(percentiles)
        assert np.allclose(actual, expected)
    if expected_significant_correlations is not None:
        expected = combine_edges(expected_significant_correlations, 'bait', 'gene')
        df_.assert_equals(significant_correlations, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    if expected_bait_nodes is not None:
        actual = bait_nodes[['genes', 'family']].copy()
        actual['gene'] = actual['genes'].apply(one)
        del actual['genes']
        df_.assert_equals(actual, expected_bait_nodes, ignore_order={0,1}, ignore_indices={0}, all_close=True)
    if expected_family_nodes is not None:
        actual = family_nodes[['genes', 'family']].copy()
        df_.assert_equals(actual, expected_family_nodes, ignore_order={0,1}, ignore_indices={0}, all_close=True)
    if expected_gene_nodes is not None:
        actual = gene_nodes[['genes']].applymap(one)
        expected = pd.DataFrame(expected_gene_nodes, columns=['genes'])
        df_.assert_equals(actual, expected, ignore_order={0,1}, ignore_indices={0}, all_close=True)
    if expected_correlation_edges is not None:
        expected = combine_edges(expected_correlation_edges, 'bait', 'gene')
        df_.assert_equals(correlation_edges, expected, ignore_order={0}, ignore_indices={0}, all_close=True)
    if partitions is not None:
        actual = set(nodes.groupby('partition_id')['label'].apply(frozenset).tolist())
        assert actual == partitions, (actual, partitions)
    if expected_homology_edges is not None:
        actual = set(homology_edges['combined'])
        expected = set(map(frozenset, expected_homology_edges))
        assert actual == expected, (actual, expected)

@pytest.fixture
def no_gene_families():
    return pd.DataFrame(columns=('family', 'gene'))

@pytest.fixture
def trivial_input(all_correlate_perfectly_input):
    '''
    Trivial input that shouldn't cause errors

    Uses just 1 matrix. Has 2 baits: bait1, bait2. Has 2 non-baits gene1, gene2.
    '''
    return all_correlate_perfectly_input

@pytest.fixture
def all_correlate_perfectly_input(no_gene_families):
    '''
    All genes correlate or anti-correlate completely.

    Uses just 1 matrix. Has 2 baits: bait1, bait2. Has 2 non-baits gene1, gene2.
    '''
    trivial_matrix = ExpressionMatrix(
        'trivial matrix',
        pd.DataFrame(
            [
                [3, 5],
                [2, 1],
                [6, 10],
                [4, 2]
            ],
            columns=['condition1', 'condition2'],
            index=['bait1', 'bait2', 'gene1', 'gene2'],
            dtype=float
        )
    )

    return dict(
        baits=pd.Series(['bait1', 'bait2']),
        expression_matrices=[trivial_matrix],
        gene_families=no_gene_families
    )

@pytest.fixture
def cutoff_input(no_gene_families):
    '''
    Input where gene2, gene3 are dropped

    gene1 and gene4 each correlate with 1 bait.

    2 baits: bait1 bait2. 4 genes: gene1 ... gene4.
    '''
    matrix = ExpressionMatrix(
        'cutoff input matrix',
        pd.DataFrame(
            [
                [0, 1, 2],
                [0, 1, 4],
                [0, 1, 0.8],
                [0, 1, 0.5],
                [0, 1, -1],
                [0, 1, -2]
            ],
            columns=['condition1', 'condition2', 'condition3'],
            index=['bait1', 'bait2', 'gene1', 'gene2', 'gene3', 'gene4'],
            dtype=float
        )
    )

    # Given that the sample size is normally 800, this is the correlation
    # matrix used to derive the percentiles. The diagonal is ignored.
    #
    #     [
    #         [1.0, 0.96076892283052284, 0.7559289460184544, 0.5, -0.5, -0.65465367070797709],
    #         [0.96076892283052284, 1.0, 0.54470477940192219, 0.24019223070763071, -0.7205766921228921, -0.83862786937753464],
    #         [0.7559289460184544, 0.54470477940192219, 1.0, 0.94491118252306794, 0.18898223650461354, -6.8677238799412995e-17],
    #         [0.5, 0.24019223070763071, 0.94491118252306794, 1.0, 0.5, 0.32732683535398854],
    #         [-0.5, -0.7205766921228921, 0.18898223650461354, 0.5, 1.0, 0.98198050606196563],
    #         [-0.65465367070797709, -0.83862786937753464, -6.8677238799412995e-17, 0.32732683535398854, 0.98198050606196563, 1.0]
    #     ]

    percentile_ranks = (4, 75.6)  # the percentile ranks that will get us the above percentiles

    return dict(
        baits=pd.Series(['bait1', 'bait2']),
        expression_matrices=[matrix],
        gene_families=no_gene_families,
        percentile_ranks=percentile_ranks
    )

@pytest.fixture
def cutoff_input_percentiles():
    '''
    Percentiles corresponding to the percentile ranks of cutoff_input
    '''
    return (-0.81973968101679184, 0.73987590935559711)

class TestCutoffs(object):

    '''
    Test lower and upper percentile rank cutoffs
    '''

    inputs = (
        (-1, False),
        (0, True),
        (0.1, True),
        (50, True),
        (50.5, True),
        (99, True),
        (100, True),
        (100.1, False)
    )

    @pytest.mark.parametrize('value,is_valid,is_lower', (x + (y,) for x,y in product(inputs, (True, False))))
    def test_input_validation(self, trivial_input, value, is_valid, is_lower):
        '''
        Error iff invalid value
        '''
        if is_lower:
            percentile_ranks = (value, 100)
        else:
            percentile_ranks = (0, value)
        trivial_input['percentile_ranks'] = percentile_ranks
        if is_valid:
            create_network(**trivial_input)
        else:
            with pytest.raises(ValueError) as ex:
                create_network(**trivial_input)
            assert 'Percentile ranks must be in range of [0, 100], got: {}'.format(np.array(percentile_ranks)) in str(ex.value)

    def test_input_validation2(self, trivial_input):
        '''
        Error if lower > upper
        '''
        trivial_input['percentile_ranks'] = (51, 50)
        with pytest.raises(ValueError) as ex:
            create_network(**trivial_input)
        assert 'Lower percentile rank must be less or equal to upper percentile rank' in str(ex.value)

        # lower == upper, is fine
        trivial_input['percentile_ranks'] = (50, 50)
        network = create_network(**trivial_input)
        assert_network(trivial_input, network)

    def test_no_cutoff(self, all_correlate_perfectly_input):
        '''
        Lower and upper cutoff of (x, x) musn't cut anything
        '''
        all_correlate_perfectly_input['percentile_ranks'] = (50, 50)
        network = create_network(**all_correlate_perfectly_input)
        assert_network(all_correlate_perfectly_input, network)

    def test_cutoff(self, cutoff_input, cutoff_input_percentiles):
        '''
        Do cut off when out of range of [lower, upper percentile]
        '''
        network = create_network(**cutoff_input)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'bait2', 0.96076892283052284],
                ['bait1', 'gene1', 0.7559289460184544],
                ['bait2', 'gene4', -0.83862786937753464],
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            cutoff_input,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame(columns=('genes', 'family')),
            gene_nodes=['gene1', 'gene4'],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            percentiles=[cutoff_input_percentiles],
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'gene1'}),
                frozenset({'gene4'}),
            }
        )

    def test_never_cut_bait_nodes(self, cutoff_input):
        '''
        Even when nothing correlates with a bait node, leave it in
        '''
        # gene2 and gene4 are normally dropped in cutoff_input, making them a bait should keep them in
        cutoff_input['baits'] = pd.Series(['bait1', 'bait2', 'gene2', 'gene4'])
        network = create_network(**cutoff_input)
        assert_network(
            cutoff_input,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                    ['gene2', None],
                    ['gene4', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame(columns=('genes', 'family')),
        )

class TestBaits(object):

    def test_empty(self, trivial_input):
        '''
        When empty, raise ValueError
        '''
        trivial_input['baits'] = pd.Series()
        with pytest.raises(ValueError) as ex:
            create_network(**trivial_input)
        assert 'Must specify at least one bait' in str(ex.value)

    def test_not_in_any_matrix(self, trivial_input):
        '''
        When a bait not in any of the matrices, raise ValueError
        '''
        trivial_input['baits'] = pd.Series(['bait_missing'])
        with pytest.raises(ValueError) as ex:
            create_network(**trivial_input)
        print(str(ex.value))
        assert (
            dedent('''\
                Each of the following baits is either missing from all or present in
                multiple expression matrices:
                
                Gene name      bait_missing
                Matrix name                
                trivial matrix       absent
                
                Missing baits are columns with no "present" value, while baits in
                multiple matrices have multiple "present" values in a column.'''
            ) in str(ex.value)
        )

class TestGeneFamilies(object):

    '''
    In various cases test that:

    - when a gene is not part of a family, make a family node with as name the gene's symbol
    - only drop a family node iff none of its genes correlate with a bait

    max_correlation takes the correlation with the max abs(correlation) when a
    fam node has multiple genes correlating with the same bait
    '''

    # Note: the no families case is already covered by many of the other tests

    def test_all(self, cutoff_input):
        '''
        When gene families cover all genes and multiple families have a correlating gene
        '''
        # Add families
        gene_families = pd.DataFrame(
            [
                ['fam1', 'bait1'],
                ['fam1', 'bait2'],
                ['fam2', 'gene1'],
                ['fam2', 'gene2'],
                ['fam3', 'gene3'],
                ['fam3', 'gene4'],
            ],
            columns=('family', 'gene')
        )
        cutoff_input['gene_families'] = gene_families

        # Note: adjusted from test_cutoff 
        network = create_network(**cutoff_input)
        assert_network(
            cutoff_input,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', 'fam1'],
                    ['bait2', 'fam1'],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame(
                [
                    [{'gene1'}, 'fam2'],
                    [{'gene4'}, 'fam3'],
                ],
                columns=['genes', 'family']
            ),
            gene_nodes=[],
            homology_edges=[['bait1', 'bait2']],
            significant_correlations=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'gene1', 0.7559289460184544],
                    ['bait2', 'gene4', -0.83862786937753464],
                ],
                columns=['bait', 'gene', 'correlation']
            ),
            correlation_edges=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'fam2', 0.7559289460184544],
                    ['bait2', 'fam3', -0.83862786937753464],
                ],
                columns=['bait', 'gene', 'max_correlation']
            ),
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'fam2'}),
                frozenset({'fam3'}),
            }
        )

    def test_all2(self, cutoff_input):
        '''
        When gene families cover all genes and a family has multiple correlating genes
        '''
        # Add families
        gene_families = pd.DataFrame(
            [
                ['fam1', 'bait1'],
                ['fam1', 'bait2'],
                ['fam2', 'gene1'],
                ['fam2', 'gene4'],
                ['fam3', 'gene2'],
                ['fam3', 'gene3'],
            ],
            columns=('family', 'gene')
        )
        cutoff_input['gene_families'] = gene_families

        # Note: adjusted from test_cutoff 
        network = create_network(**cutoff_input)
        assert_network(
            cutoff_input,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', 'fam1'],
                    ['bait2', 'fam1'],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame(
                [
                    [{'gene1', 'gene4'}, 'fam2'],
                ],
                columns=['genes', 'family']
            ),
            gene_nodes=[],
            homology_edges=[['bait1', 'bait2']],
            significant_correlations=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'gene1', 0.7559289460184544],
                    ['bait2', 'gene4', -0.83862786937753464],
                ],
                columns=['bait', 'gene', 'correlation']
            ),
            correlation_edges=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'fam2', 0.7559289460184544],
                    ['bait2', 'fam2', -0.83862786937753464],
                ],
                columns=['bait', 'gene', 'max_correlation']
            ),
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'fam2'}),
            }
        )

    def test_some(self, cutoff_input):
        '''
        When gene families cover only some of the genes, run fine
        '''
        # Add families
        gene_families = pd.DataFrame(
            [
                ['fam1', 'bait1'],
                ['fam2', 'gene1'],
            ],
            columns=('family', 'gene')
        )
        cutoff_input['gene_families'] = gene_families

        # Note: adjusted from test_cutoff 
        network = create_network(**cutoff_input)
        assert_network(
            cutoff_input,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', 'fam1'],
                    ['bait2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame(
                [
                    [{'gene1'}, 'fam2'],
                ],
                columns=['genes', 'family']
            ),
            gene_nodes=['gene4'],
            homology_edges=[],
            significant_correlations=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'gene1', 0.7559289460184544],
                    ['bait2', 'gene4', -0.83862786937753464],
                ],
                columns=['bait', 'gene', 'correlation']
            ),
            correlation_edges=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'fam2', 0.7559289460184544],
                    ['bait2', 'gene4', -0.83862786937753464],
                ],
                columns=['bait', 'gene', 'max_correlation']
            ),
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'fam2'}),
                frozenset({'gene4'}),
            }
        )

    def test_max_correlation(self, cutoff_input):
        '''
        When a family has multiple genes correlating with the same bait, correct max_correlation
        '''
        # Add families
        gene_families = pd.DataFrame(
            [
                ['fam1', 'bait1'],
                ['fam1', 'bait2'],
                ['fam2', 'gene1'],
                ['fam2', 'gene2'],
                ['fam3', 'gene3'],
                ['fam3', 'gene4'],
            ],
            columns=('family', 'gene')
        )
        cutoff_input['gene_families'] = gene_families

        # Note: adjusted from test_cutoff
        cutoff_input['percentile_ranks'] = (50, 50)  # no cutoff
        network = create_network(**cutoff_input)
        assert_network(
            cutoff_input,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', 'fam1'],
                    ['bait2', 'fam1'],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame(
                [
                    [{'gene1', 'gene2'}, 'fam2'],
                    [{'gene3', 'gene4'}, 'fam3'],
                ],
                columns=['genes', 'family']
            ),
            gene_nodes=[],
            homology_edges=[['bait1', 'bait2']],
            significant_correlations=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'gene1', 0.7559289460184544],
                    ['bait1', 'gene2', 0.5],
                    ['bait1', 'gene3', -0.5],
                    ['bait1', 'gene4', -0.65465367070797709],
                    ['bait2', 'gene1', 0.54470477940192219],
                    ['bait2', 'gene2', 0.24019223070763071],
                    ['bait2', 'gene3', -0.7205766921228921],
                    ['bait2', 'gene4', -0.83862786937753464]
                ],
                columns=['bait', 'gene', 'correlation']
            ),
            correlation_edges=pd.DataFrame(
                [
                    ['bait1', 'bait2', 0.96076892283052284],
                    ['bait1', 'fam2', 0.7559289460184544],
                    ['bait1', 'fam3', -0.65465367070797709],
                    ['bait2', 'fam2', 0.54470477940192219],
                    ['bait2', 'fam3', -0.83862786937753464]
                ],
                columns=['bait', 'gene', 'max_correlation']
            ),
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'fam2', 'fam3'}),
            }
        )

class TestExpressionMatrices(object):

    @pytest.fixture
    def expression_matrix(self):
        return pd.DataFrame(
            [
                [0, 1, 2],
                [0, 1, 4],
                [0, 1, 0.8],
                [0, 1, 0.5],
            ],
            columns=['condition1', 'condition2', 'condition3'],
            index=['bait1', 'bait2', 'gene1', 'gene2'],
            dtype=float
        )

    @pytest.fixture
    def correlation_matrix(self):
        '''
        All vs all correlation matrix of `expression_matrix`
        '''
        genes = pd.Series(['bait1', 'bait2', 'gene1', 'gene2'])
        return pd.DataFrame(  # full correlation matrix, as if all were baits. Pick what you need
            [
                [1.0, 0.96076892283052284, 0.7559289460184544, 0.5],
                [0.96076892283052284, 1.0, 0.54470477940192219, 0.24019223070763071],
                [0.7559289460184544, 0.54470477940192219, 1.0, 0.94491118252306794],
                [0.5, 0.24019223070763071, 0.94491118252306794, 1.0],
            ],
            columns=genes,
            index=genes,
            dtype=float
        )

    def test_empty_list(self, trivial_input):
        trivial_input['expression_matrices'] = []
        with pytest.raises(ValueError) as ex:
            create_network(**trivial_input)
        assert 'Must provide at least one expression matrix' in str(ex.value)

    def test_no_bait_genes(self, trivial_input):
        '''
        When a matrix has no baits, raise ValueError
        '''
        matrix = ExpressionMatrix(
            'matrix',
            pd.DataFrame(
                [[1,2], [3,4]],
                columns=['condition1', 'condition2'],
                index=['gene5', 'gene6'],
                dtype=float
            )
        )
        trivial_input['expression_matrices'].append(matrix)
        with pytest.raises(ValueError) as ex:
            create_network(**trivial_input)
        expected = (
            'Some expression matrices have no baits: {}. '
            'Each expression matrix must contain at least one bait. '
            'Either drop the matrices or add some of their genes to the baits list.'
            .format(matrix)
        )
        assert expected in str(ex.value), '\nActual\n{}\n\nExpected\n{}\n'.format(str(ex.value), expected)

    def test_no_non_bait_genes(self, expression_matrix, correlation_matrix, no_gene_families):
        '''
        When all rows are baits, run fine
        '''
        matrix = ExpressionMatrix('matrix', expression_matrix)
        input_ = dict(
            baits=pd.Series(['bait1', 'bait2', 'gene1', 'gene2']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
            percentile_ranks=(14.6, 85)
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'bait2', 0.96076892283052284],
                ['bait2', 'gene2', 0.24019223070763071]
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_, 
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                    ['gene1', None],
                    ['gene2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=[],
            samples=[correlation_matrix],
            correlation_matrices=[correlation_matrix],
            percentiles=[(0.39763573889880643, 0.95046139163067722)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1', 'bait2', 'gene1', 'gene2'}),
            }
        )

    def test_1_non_bait_gene_cor_1_bait(self, expression_matrix, correlation_matrix, no_gene_families):
        '''
        When matrix has 1 non-bait which correlates with 1 bait, run fine
        '''
        matrix = ExpressionMatrix('matrix', expression_matrix.iloc[[0, 2]])
        input_ = dict(
            baits=pd.Series(['bait1']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
            percentile_ranks=(20, 80)
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'gene1', 0.7559289460184544]
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_, 
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=['gene1'],
            samples=[correlation_matrix.iloc[[0,2],[0,2]]],
            correlation_matrices=[correlation_matrix.iloc[[0,2], [0]]],
            percentiles=[(0.7559289460184544, 0.7559289460184544)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1'}),
                frozenset({'gene1'}),
            }
        )

    def test_1_non_bait_gene_cor_2_baits(self, expression_matrix, correlation_matrix, no_gene_families):
        '''
        When matrix has 1 non-bait which correlates with 2 baits, run fine
        '''
        matrix = ExpressionMatrix('matrix', expression_matrix.iloc[[0,1,2]])
        input_ = dict(
            baits=pd.Series(['bait1', 'bait2']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
            percentile_ranks=(0, 20)
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'bait2', 0.96076892283052284],
                ['bait1', 'gene1', 0.7559289460184544],
                ['bait2', 'gene1', 0.54470477940192219]
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_, 
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=['gene1'],
            samples=[correlation_matrix.iloc[[0,1,2], [0,1,2]]],
            correlation_matrices=[correlation_matrix.iloc[[0,1,2], [0,1]]],
            percentiles=[(0.54470477940192219, 0.54470477940192219)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'gene1'}),
            }
        )

    def test_2_non_bait_genes_cor_1_bait(self, expression_matrix, correlation_matrix, no_gene_families):
        '''
        When matrix has 2 non-baits which correlate with 1 bait, run fine
        '''
        matrix = ExpressionMatrix('matrix', expression_matrix.iloc[[0,2,3]])
        input_ = dict(
            baits=pd.Series(['bait1']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
            percentile_ranks=(30, 40)
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'gene1', 0.7559289460184544],
                ['bait1', 'gene2', 0.5]
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_, 
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=['gene1', 'gene2'],
            samples=[correlation_matrix.iloc[[0,2,3],[0,2,3]]],
            correlation_matrices=[correlation_matrix.iloc[[0,2,3], [0]]],
            percentiles=[(0.6279644730092272, 0.7559289460184544)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1'}),
                frozenset({'gene1', 'gene2'}),
            }
        )

    def test_2_non_bait_genes_cor_2_baits(self, expression_matrix, correlation_matrix, no_gene_families):
        '''
        When matrix has 2 non-baits which correlate with 2 baits, run fine
        '''
        matrix = ExpressionMatrix('matrix', expression_matrix)
        input_ = dict(
            baits=pd.Series(['bait1', 'bait2']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
            percentile_ranks=(50, 50)
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'bait2', 0.96076892283052284],
                ['bait1', 'gene1', 0.7559289460184544],
                ['bait2', 'gene1', 0.54470477940192219],
                ['bait1', 'gene2', 0.5],
                ['bait2', 'gene2', 0.24019223070763071],
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_, 
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=['gene1', 'gene2'],
            samples=[correlation_matrix],
            correlation_matrices=[correlation_matrix.iloc[:,[0,1]]],
            percentiles=[(0.65031686271018829, 0.65031686271018829)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'gene1', 'gene2'}),
            }
        )

    def test_none_correlate(self, expression_matrix, correlation_matrix, no_gene_families):
        '''
        When none of the genes correlate with any bait, run fine
        '''
        matrix = ExpressionMatrix('matrix', expression_matrix)
        input_ = dict(
            baits=pd.Series(['bait1', 'bait2', 'gene2']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
            percentile_ranks=(0, 100)
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'bait2', 0.96076892283052284],
                ['bait2', 'gene2', 0.24019223070763071],
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_, 
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                    ['gene2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=[],
            samples=[correlation_matrix],
            correlation_matrices=[correlation_matrix.iloc[:, [0, 1, 3]]],
            percentiles=[(0.24019223070763071, 0.96076892283052284)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1', 'bait2', 'gene2'}),
            }
        )

    def test_zero_matrix(self, no_gene_families):
        '''
        When matrix with 0-variance across all rows, more genes than baits, raise ValueError
        '''
        matrix = ExpressionMatrix(
            'matrix',
            pd.DataFrame(np.zeros([2,2]), index=['bait1', 'gene1'])
        )
        input_ = dict(
            baits=pd.Series(['bait1']),
            expression_matrices=[matrix],
            gene_families=no_gene_families,
        )
        with pytest.raises(ValueError) as ex:
            create_network(**input_)
        assert 'After dropping rows with tiny standard deviation, {} has no rows.'.format(matrix) in str(ex.value)

    def test_multiple_matrices(self, no_gene_families):
        '''
        When 2 matrices, use both
        '''
        genes = pd.Series(['bait1', 'bait2', 'gene1', 'gene2'])
        matrices = [
            ExpressionMatrix(
                'matrix1',
                pd.DataFrame(
                    [
                        [0, 1, 2],
                        [0, 1, 0.8],
                    ],
                    index=['bait1', 'gene1'],
                    dtype=float
                ),
            ),
            ExpressionMatrix(
                'matrix2',
                pd.DataFrame(
                    [
                        [0, 1, 4],
                        [0, 1, 0.5],
                    ],
                    index=['bait2', 'gene2'],
                    dtype=float
                ),
            ),
        ]

        samples = [
            pd.DataFrame(
                [
                    [1.0, 0.7559289460184544],
                    [0.7559289460184544, 1.0],
                ],
                index=genes.iloc[[0,2]],
                columns=genes.iloc[[0,2]],
            ),    
            pd.DataFrame(
                [
                    [1.0, 0.24019223070763071],
                    [0.24019223070763071, 1.0],
                ],
                index=genes.iloc[[1,3]],
                columns=genes.iloc[[1,3]],
            )
        ]

        correlation_matrices = [
            samples[0].iloc[:,[0]],
            samples[1].iloc[:,[0]],
        ]

        input_ = dict(
            baits=pd.Series(['bait1', 'bait2']),
            expression_matrices=matrices,
            gene_families=no_gene_families,
        )
        network = create_network(**input_)
        significant_correlations = pd.DataFrame(
            [
                ['bait1', 'gene1', 0.7559289460184544],
                ['bait2', 'gene2', 0.24019223070763071],
            ],
            columns=['bait', 'gene', 'correlation']
        )
        correlation_edges = significant_correlations.rename(columns={'correlation' : 'max_correlation'})
        assert_network(
            input_,
            network,
            bait_nodes=pd.DataFrame(
                [
                    ['bait1', None],
                    ['bait2', None],
                ],
                columns=['gene', 'family']
            ),
            family_nodes=pd.DataFrame([], columns=['genes', 'family']),
            gene_nodes=['gene1', 'gene2'],
            samples=samples,
            correlation_matrices=correlation_matrices,
            percentiles=[(0.7559289460184544, 0.7559289460184544), (0.24019223070763071, 0.24019223070763071)],
            homology_edges=pd.DataFrame(),
            significant_correlations=significant_correlations,
            correlation_edges=correlation_edges,
            partitions={
                frozenset({'bait1', 'bait2'}),
                frozenset({'gene1'}),
                frozenset({'gene2'}),
            }
        )

    @pytest.mark.parametrize('is_bait', (True, False))
    def test_overlap(self, is_bait, no_gene_families):
        '''
        When 2 matrices' genes overlap, raise ValueError
        '''
        matrix1 = ExpressionMatrix(
            'matrix1',
            pd.DataFrame(
                [[1,2],[3,4],[5,6]],
                columns=['condition1', 'condition2'],
                index=['bait1', 'gene1', 'gene_overlapping'],
                dtype=float
            )
        )
        matrix2 = ExpressionMatrix(
            'matrix2',
            pd.DataFrame(
                [[1,2],[3,4],[5,6]],
                columns=['condition1', 'condition2'],
                index=['bait2', 'gene2', 'gene_overlapping'],
                dtype=float
            )
        )
        baits = ['bait1', 'bait2']
        if is_bait:
            baits.append('gene_overlapping')
        with pytest.raises(ValueError) as ex:
            create_network(
                baits=pd.Series(baits),
                expression_matrices=[matrix1, matrix2],
                gene_families=no_gene_families,
            )
        print(str(ex.value))
        if is_bait:
            assert (
                dedent('''\
                    Each of the following baits is either missing from all or present in
                    multiple expression matrices:
                    
                    Gene name   gene_overlapping
                    Matrix name                 
                    matrix1              present
                    matrix2              present
                    
                    Missing baits are columns with no "present" value, while baits in
                    multiple matrices have multiple "present" values in a column.'''
                ) in str(ex.value)
            )
        else:
            assert (
                'The following genes appear in multiple expression matrices: gene_overlapping. '
                'CoExpNetViz does not support gene expression data from different matrices for the same gene. '
                'Please remove rows from the given matrices such that no gene appears in multiple matrices.'
                in str(ex.value)
            )
 
class TestCorrelationMethod(object):

    '''
    Test whether the correct correlation method is used and test that at least
    it works with pearson and mutual_information
    '''

    @pytest.fixture
    def input_(self, no_gene_families):
        matrix = ExpressionMatrix(
            'matrix',
            pd.DataFrame(
                [
                    [1, 2, 3],
                    [3, 2, 1],
                    [1, 2, 1],
                    [8, 3, 5]
                ],
                index=['gene1', 'gene2', 'gene3', 'gene4'],
                dtype=float
            )
        )
        return dict(
            baits=pd.Series(['gene1', 'gene2']),
            expression_matrices=[matrix],
            gene_families=no_gene_families
        )

    @pytest.fixture
    def genes(self):
        return pd.Series(['gene1', 'gene2', 'gene3', 'gene4'])

    def test_pearson(self, input_, genes):
        input_['correlation_function'] = correlation.pearson_df
        network = create_network(**input_)
        assert_network(
            input_,
            network,
            correlation_matrices=[pd.DataFrame(
                [
                    [1, -1],
                    [-1, 1],
                    [0, 0],
                    [-0.59603956067926978, 0.59603956067926978]
                ],
                index=genes,
                columns=genes.iloc[[0,1]],
                dtype=float
            )]
        )

    def test_mutual_information(self, input_, genes):
        input_['correlation_function'] = correlation.mutual_information_df
        network = create_network(**input_)
        assert_network(
            input_,
            network,
            correlation_matrices=[pd.DataFrame(
                [
                    [1.0986122886681096, 1.0986122886681096],
                    [1.0986122886681096, 1.0986122886681096],
                    [0.63651416829481289, 0.63651416829481289],
                    [1.0986122886681096, 1.0986122886681096]
                ],
                index=genes,
                columns=genes.iloc[[0,1]],
                dtype=float
            )]
        )

# TODO test that when duplicate baits, raise ValueError.

# TODO what if a family node is created of some non-bait gene but one of
# the baits is also present in that family! Probably it should tell the user to
# include said gene as a bait
