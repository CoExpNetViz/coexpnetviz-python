# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

import pandas as pd
from pkg_resources import resource_string  # @UnresolvedImport
from more_itertools import one
from ._various import NodeType

def write_cytoscape(network, name):
    '''
    Write a cytoscape network

    Creates the following files:

    $name.sif
        The network structure: list of nodes and the edges between them.
    $name.node.attr
        Node attributes
    $name.edge.attr
        Edge attributes
    coexpnetviz_style.xml
        Style to display the network with

    Parameters
    ----------
    network : Network
        The network to write
    name : str
        Name of the network.
    '''
    if network.nodes.empty:
        raise ValueError('network.nodes is empty. Cytoscape networks must have at least one node.')

    _write_node_attr(network, name)
    _write_edge_attr(network, name)
    _write_sif(network, name)

    # Copy additional files
    # Note: can't do a plain file copy as resource might be in an egg
    with open('coexpnetviz_style.xml', 'wb') as f:
        f.write(resource_string('coexpnetviz', 'data/coexpnetviz_style.xml'))

def _write_node_attr(network, network_name):
    nodes = network.nodes.copy()

    # bait nodes
    bait_nodes = nodes[nodes['type'] == NodeType.bait]
    nodes['bait_gene'] = bait_nodes['genes'].apply(one)
    nodes['families'] = bait_nodes['family']

    # family/gene nodes
    family_nodes = nodes[nodes['type'] != NodeType.bait]
    nodes['family'] = family_nodes['family']
    nodes['correlating_genes_in_family'] = family_nodes['genes'].apply(lambda genes: ', '.join(sorted(genes)))

    # gene nodes
    gene_nodes = nodes[nodes['type'] == NodeType.gene]
    nodes['family'].update(gene_nodes['correlating_genes_in_family'])

    # any node
    nodes['id'] = nodes['id'].apply(_format_node_id)
    nodes['type'] = nodes['type'].apply(lambda x: 'bait node' if x == NodeType.bait else 'family node')
    nodes['colour'] = nodes['colour'].apply(lambda x: x.to_hex())
    nodes['species'] = None
    del nodes['genes']

    # write
    nodes = nodes.reindex(columns=('id', 'label', 'colour', 'type', 'bait_gene', 'species', 'families', 'family', 'correlating_genes_in_family', 'partition_id'))
    nodes.to_csv('{}.node.attr'.format(network_name), sep='\t', index=False)

def _write_edge_attr(network, network_name):
    if network.correlation_edges.empty:
        return
    edges = network.correlation_edges.copy()
    edges.insert(0, 'edge', edges[['bait_node', 'node']].applymap(_format_node_id).apply(lambda nodes: '{} (cor) {}'.format(*nodes), axis=1))
    del edges['bait_node']
    del edges['node']
    edges.to_csv('{}.edge.attr'.format(network_name), sep='\t', index=False)

def _write_sif(network, network_name):
    edges = []

    # bait nodes
    nodes = network.nodes
    edges_ = nodes[nodes.type == NodeType.bait]['id'].to_frame('node1')
    edges.append(edges_)

    # correlation edges
    if not network.correlation_edges.empty:
        edges_ = network.correlation_edges[['bait_node', 'node']].copy()
        edges_.columns = ('node1', 'node2')
        edges_['type'] = 'cor'
        edges.append(edges_)

    # homology edges
    if not network.homology_edges.empty:
        edges_ = network.homology_edges.copy()
        edges_.columns = ('node1', 'node2')
        edges_['type'] = 'hom'
        edges.append(edges_)

    # concat
    sif = pd.concat(edges, ignore_index=True)
    sif = sif.reindex(columns=('node1', 'type', 'node2'))
    sif[['node1', 'node2']] = sif[['node1', 'node2']].applymap(_format_node_id)
    sif.to_csv('{}.sif'.format(network_name), sep='\t', header=False, index=False)

def _format_node_id(node_id):
    if pd.isnull(node_id):
        return ''
    else:
        return 'n{}'.format(int(node_id))