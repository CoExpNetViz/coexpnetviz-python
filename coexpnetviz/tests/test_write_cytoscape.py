# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <tim@diels.me>
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
Test coexpnetviz.write_cytoscape()

I.e. test written Cytoscape files are correctly formatted and match the input Network
'''

from pathlib import Path
from pkg_resources import resource_string  # @UnresolvedImport

from pytil.data_frame import assert_df_equals, replace_na_with_none
import attr
import numpy as np
import pandas as pd
import pytest

from coexpnetviz import NodeType, write_cytoscape, Network, RGB
from coexpnetviz._various import MutableNetwork


@pytest.fixture(autouse=True)
def use_temp_dir_cwd(temp_dir_cwd):
    pass

def assert_(network):
    '''
    Assert that written files match network

    $name.node.attr:

    - id is prefixed with 'n'
    - label is kept unchanged
    - type is 'bait' iff bait, 'family' otherwise
    - genes is ', '.join(sorted(gene))
    - family is '' if null else family name
    - colour is #FFFFFF
    - partition_id is kept unchanged
    - no duplicate rows, tab separated columns

    $name.edge.attr:

    - no homology edges
    - correlation edges with max_correlation as value
    - no duplicate rows, tab separated columns

    $name.sif:

    - correlation edges
    - homology edges
    - each node is present in either first or third column
    - no duplicate rows, tab separated columns

    coexpnetviz_style.xml matches contents of data/coexpnetviz_style.xml

    Parameters
    ----------
    network
        Only the network of the `network` fixture, optionally with
        homology_edges and/or correlation_edges made empty
    '''
    network_name = 'amazing network'
    write_cytoscape(Network(**attr.asdict(network)), network_name, output_dir=Path())

    node_attr_file = Path(network_name + '.node.attr')
    edge_attr_file = Path(network_name + '.edge.attr')
    sif_file = Path(network_name + '.sif')

    # node.attr
    node_attr = pd.read_table(str(node_attr_file))
    expected = pd.DataFrame(
        [
            ['n2', 'labeL2', '#FFFFFF', 'bait node', 'bait1', None, 'fam1', None, None, 10],
            ['n9', 'label9', '#FF0000', 'family node', None, None, None, 'gene5', 'gene5', 18],
            ['n8', 'label8', '#0000FF', 'family node', None, None, None, 'fam2', 'gene3', 11],
            ['n22', 'label22', '#00FF00', 'family node', None, None, None, 'gene4', 'gene4', 14],
            ['n3', 'label3', '#FFFFFF', 'bait node', 'bait2', None, 'fam1', None, None, 10],
            ['n5', 'label5', '#0000FF', 'family node', None, None, None, 'fam1', 'gene1, gene2', 11],
            ['n20', 'label20', '#FFFFFF', 'bait node', 'bait3', None, None, None, None, 10],
        ],
        columns=('id', 'label', 'colour', 'type', 'bait_gene', 'species', 'families', 'family', 'correlating_genes_in_family', 'partition_id')
    )
    node_attr['colour'] = node_attr['colour'].str.upper()
    assert_df_equals(node_attr, expected, ignore_indices={0,1}, ignore_order={0})

    # edge.attr
    if network.correlation_edges.empty:
        assert not edge_attr_file.exists()
    else:
        edge_attr = pd.read_table(str(edge_attr_file))
        edges = edge_attr['edge'].apply(lambda x: pd.Series(x.split()))
        del edge_attr['edge']
        edge_attr['relation'] = edges[1]
        edge_attr['nodes'] = edges[[0,2]].apply(frozenset, axis=1)
        expected = pd.DataFrame(
            [
                [{'n2', 'n20'}, '(cor)', 0.9],
                [{'n3', 'n5'}, '(cor)', -0.8],
            ],
            columns=('nodes', 'relation', 'max_correlation')
        )
        expected['nodes'] = expected['nodes'].apply(frozenset)
        assert_df_equals(edge_attr, expected, ignore_indices={0,1}, ignore_order={0,1})

    # sif
    expected = [
        [{'n2', None}, np.nan],
        [{'n3', None}, np.nan],
        [{'n20', None}, np.nan],
    ]
    if not network.correlation_edges.empty:
        expected.extend([
            [{'n2', 'n20'}, 'cor'],
            [{'n3', 'n5'}, 'cor'],
        ])
    if not network.homology_edges.empty:
        expected.append([{'n2', 'n3'}, 'hom'])
    expected = pd.DataFrame(expected)
    expected[0] = expected[0].apply(frozenset)
    actual = pd.read_table(str(sif_file), header=None)
    actual = replace_na_with_none(actual)
    actual[0] = actual[[0,2]].apply(frozenset, axis=1)
    del actual[2]
    assert_df_equals(actual, expected, ignore_indices={0,1}, ignore_order={0,1})

    # other files copied verbatim
    actual = resource_string('coexpnetviz', 'data/coexpnetviz_style.xml')
    expected = Path('coexpnetviz_style.xml').read_bytes()
    assert actual == expected

@pytest.fixture
def network():
    nodes = pd.DataFrame([
            [2, 'labeL2', NodeType.bait, {'bait1'}, 'fam1', RGB((255,255,255)), 10],
            [9, 'label9', NodeType.gene, {'gene5'}, None, RGB((255,0,0)), 18],
            [8, 'label8', NodeType.family, {'gene3'}, 'fam2', RGB((0,0,255)), 11],
            [22, 'label22', NodeType.gene, {'gene4'}, np.nan, RGB((0,255,0)), 14],
            [3, 'label3', NodeType.bait, {'bait2'}, 'fam1', RGB((255,255,255)), 10],
            [5, 'label5', NodeType.family, {'gene1', 'gene2'}, 'fam1', RGB((0,0,255)), 11],
            [20, 'label20', NodeType.bait, {'bait3'}, None, RGB((255,255,255)), 10],
        ],
        columns=('id', 'label', 'type', 'genes', 'family', 'colour', 'partition_id')
    )
    nodes['id'] = nodes['id'].astype(int)
    nodes['partition_id'] = nodes['partition_id'].astype(int)
    nodes['genes'] = nodes['genes'].apply(frozenset)

    return MutableNetwork(
        nodes=nodes,
        homology_edges=pd.DataFrame([[2, 3]], columns=('bait_node1', 'bait_node2')),
        correlation_edges=pd.DataFrame(
            [
                [2, 20, 0.9],
                [3, 5, -0.8],
            ], 
            columns=('bait_node', 'node', 'max_correlation')
        ),

        # omitting these, just mocking part of a network
        significant_correlations=None,
        samples=None,
        percentiles=None,
        correlation_matrices=None
    )

def test_happy_days(network):
    '''
    For happy days input, check all output
    '''
    assert_(network)

def test_empty_network(network):
    '''
    If network has no nodes, raise ValueError
    '''
    network.nodes = pd.DataFrame(columns=network.nodes.columns)
    with pytest.raises(ValueError) as ex:
        write_cytoscape(network, 'name', Path())
    assert 'network.nodes is empty. Cytoscape networks must have at least one node.' in str(ex.value)

def test_empty_homology_edges(network):
    '''
    If network has no homology edges, run fine
    '''
    network.homology_edges = pd.DataFrame(columns=network.homology_edges.columns)
    assert_(network)

def test_empty_correlation_edges(network):
    '''
    If network has no correlation edges, run fine
    '''
    network.correlation_edges = pd.DataFrame(columns=network.correlation_edges.columns)
    assert_(network)

def test_empty_edges(network):
    '''
    If network has no edges at all, run fine
    '''
    network.homology_edges = pd.DataFrame(columns=network.homology_edges.columns)
    network.correlation_edges = pd.DataFrame(columns=network.correlation_edges.columns)
    assert_(network)
