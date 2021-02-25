# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <tim@diels.me>
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


def write_cytoscape(network, name, output_dir):
    '''
    Write a Cytoscape network.

    Parameters
    ----------
    network : Network
        The network to write.
    name : str
        Name of the network.
    output_dir : ~pathlib.Path
        Directory to write the network files to. The directory must already
        exist.
    '''

    def write_edge_attr():
        if network.cor_edges.empty:
            return
        edges = network.cor_edges.copy()
        edge_attrs = (
            edges[['bait_node', 'node']]
            .applymap(_format_node_id)
            .apply(lambda nodes: '{} (cor) {}'.format(*nodes), axis=1)
        )
        edges.insert(0, 'edge', edge_attrs)
        del edges['bait_node']
        del edges['node']
        edges.to_csv(str(output_dir / f'{name}.edge.attr'), sep='\t', index=False)

    def write_sif():
        edges = []

        # Bait nodes
        nodes = network.nodes
        edges_ = nodes[nodes.type == 'bait']['id'].to_frame('node1')
        edges.append(edges_)

        # Correlation edges
        if not network.cor_edges.empty:
            edges_ = network.cor_edges[['bait_node', 'node']].copy()
            edges_.columns = ('node1', 'node2')
            edges_['type'] = 'cor'
            edges.append(edges_)

        # Homology edges
        if not network.homology_edges.empty:
            edges_ = network.homology_edges.copy()
            edges_.columns = ('node1', 'node2')
            edges_['type'] = 'hom'
            edges.append(edges_)

        # Concat
        sif = pd.concat(edges, ignore_index=True)
        sif = sif.reindex(columns=('node1', 'type', 'node2'))
        sif[['node1', 'node2']] = sif[['node1', 'node2']].applymap(_format_node_id)
        sif.to_csv(str(output_dir / f'{name}.sif'), sep='\t', header=False, index=False)

    if network.nodes.empty:
        raise ValueError('network.nodes is empty. Cytoscape networks must have at least one node.')

    write_edge_attr()
    write_sif()

def _format_node_id(node_id):
    if pd.isnull(node_id):
        return ''
    else:
        return f'n{int(node_id)}'
