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
Test deep_genome.coexpnetviz.write_cytoscape()
 
I.e. test written Cytoscape files are correctly formatted and match the input Network
'''

import pytest
import pandas as pd
import numpy as np
from pathlib import Path
from chicken_turtle_util import series as series_, data_frame as df_, path as path_
from chicken_turtle_util.test import temp_dir_cwd
from deep_genome.coexpnetviz._various import MutableNetwork
from deep_genome.coexpnetviz import NodeType, write_cytoscape, Network, RGB
from pkg_resources import resource_string

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
    - genes is ', '.join(sorted(gene.name))
    - family is '' if null else family.name
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
    write_cytoscape(Network(**network._asdict()), network_name)
    
    node_attr_file = Path(network_name + '.node.attr')
    edge_attr_file = Path(network_name + '.edge.attr')
    sif_file = Path(network_name + '.sif')
    
    # node.attr
    node_attr = pd.read_table(str(node_attr_file))
    expected = pd.DataFrame(
#         [
#             ['n2', 'labeL2', '#FFFFFF', 'bait node', 'bait1', 10],
#             ['n9', 'label9', '#FF0000', 'family node', 'gene5', 18],
#             ['n8', 'label8', '#0000FF', 'family node', 'gene3', 11],
#             ['n22', 'label22', '#00FF00', 'family node', 'gene4', 14],
#             ['n3', 'label3', '#FFFFFF', 'bait node', 'bait2', 10],
#             ['n5', 'label5', '#0000FF', 'family node', 'gene1, gene2', 11],
#             ['n20', 'label20', '#FFFFFF', 'bait node', 'bait3', 10],
#         ],
#         columns=('id', 'label', 'color', 'type', 'genes', 'partition_id') #TODO forgot family
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
        #TODO in a release after this one, together with adjusting Cytoscape plugin:
        # - only these in the future (after cytoscape plugin has been updated). 'id', 'label', 'type', 'genes', 'family', 'color', 'partition_id'
        # - introduce gene nodes in style and plugin
        # - don't add ' node' suffix
    )
    node_attr['colour'] = node_attr['colour'].str.upper()
    df_.assert_equals(node_attr, expected, ignore_indices={0,1}, ignore_order={0})
    
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
        df_.assert_equals(edge_attr, expected, ignore_indices={0,1}, ignore_order={0,1})
        
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
    actual[2] = actual[2].astype(object)  # Note: setting None on dtype=float sets np.nan instead
    actual[2][actual[2].isnull()] = None
    actual[0] = actual[[0,2]].apply(frozenset, axis=1)
    del actual[2]
    df_.assert_equals(actual, expected, ignore_indices={0,1}, ignore_order={0,1})
    
    # other files copied verbatim
    assert resource_string('deep_genome.coexpnetviz', 'data/coexpnetviz_style.xml').decode('utf-8') == path_.read(Path('coexpnetviz_style.xml'))
    
@pytest.fixture
def network(session):
    genes = series_.split(session.get_genes_by_name(pd.Series(['bait1', 'bait2', 'gene1', 'gene2', 'gene3', 'gene4', 'gene5', 'bait3'])))
    session.add_gene_families(pd.DataFrame([
            ['fam1', 'bait1'],
            ['fam1', 'bait2'],
            ['fam1', 'gene1'],
            ['fam1', 'gene2'],
            ['fam2', 'gene3'],
        ],
        columns=('family', 'gene')
    ))

    nodes = pd.DataFrame([
            [2, 'labeL2', NodeType.bait, genes.iloc[[0]], RGB((255,255,255)), 10],
            [9, 'label9', NodeType.gene, genes.iloc[[6]], RGB((255,0,0)), 18],
            [8, 'label8', NodeType.family, genes.iloc[[4]], RGB((0,0,255)), 11],
            [22, 'label22', NodeType.gene, genes.iloc[[5]], RGB((0,255,0)), 14],
            [3, 'label3', NodeType.bait, genes.iloc[[1]], RGB((255,255,255)), 10],
            [5, 'label5', NodeType.family, genes.iloc[[2,3]], RGB((0,0,255)), 11],
            [20, 'label20', NodeType.bait, genes.iloc[[7]], RGB((255,255,255)), 10],
        ],
        columns=('id', 'label', 'type', 'genes', 'colour', 'partition_id')
    )
    nodes['id'] = nodes['id'].astype(int)
    nodes['partition_id'] = nodes['partition_id'].astype(int)
    nodes['genes'] = nodes['genes'].apply(frozenset)
    families = session.get_gene_families_by_gene(genes.iloc[[0, 4]])['family']
    nodes.insert(4, 'family', (families.iloc[0], None, families.iloc[1], np.nan, families.iloc[0], families.iloc[0], None))
    
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
        write_cytoscape(network, 'name')
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
    
    