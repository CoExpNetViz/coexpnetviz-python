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
from deep_blue_genome.core.util import series_invert, get_distinct_colours
from deep_blue_genome.pkg_util import get_data_file


# TODO consider one of the formats that allows specifying the network, edges with attributes and nodes with attributes all at once. Perhaps even with a style.
# e.g. XGMLL from http://wiki.cytoscape.org/Cytoscape_3/UserManual#Cytoscape_3.2BAC8-UserManual.2BAC8-Network_Formats.XGMML_Format
class CytoscapeWriter(object):
    def __init__(self, network):
        self._network = network
        
    def write(self):
        # assign node ids to baits: pd.Series(index=(gene : str), data=(id : int))
        self._bait_nodes = self._network.baits.copy()
        self._bait_nodes.index.name = 'id'
        self._bait_nodes = series_invert(self._bait_nodes)
        
        # assign node ids to family nodes that have a non-nan family
        next_id = self._bait_nodes.max() + 1
        family_ids = self._network.correlations['family']
        family_ids.dropna(inplace=True)
        family_ids.drop_duplicates(inplace=True)
        family_ids = family_ids.to_frame()
        family_ids['family_id'] = range(next_id, next_id+len(family_ids))
        next_id += len(family_ids)
        family_ids.set_index('family', inplace=True)
        self._correlations = self._network.correlations.set_index('family').join(family_ids)
        self._correlations.reset_index(inplace=True)
        del family_ids
        
        # assign node ids to family nodes with nan family
        mask = self._correlations['family_id'].isnull()
        self._correlations.loc[mask, 'family_id'] = range(next_id, next_id + mask.sum())
        
        # throw in a bait_id column
        # self._correlations : pd.DataFrame(columns=[(bait : str), (family : str), (family_gene : str), (correlation : float), (family_id : int), (bait_id : int)])
        self._correlations.set_index('bait', inplace=True)
        self._correlations = self._correlations.join(self._bait_nodes)
        self._correlations.reset_index(inplace=True)
        self._correlations.rename(columns={'index': 'bait', 'id': 'bait_id'}, inplace=True)
        
        # group correlation and family_gene
        # self._correlations : pd.DataFrame(columns=[(bait : str), (family : str), (family_id : int), (bait_id : int), (correlating_genes_in_family : str), (max_correlation : float)]) 
        max_correlations = self._correlations.groupby(['family_id', 'bait_id'])['correlation'].max()
        max_correlations.name = 'max_correlation'
        self._correlations.drop('correlation', axis=1, inplace=True)
        self._correlations.drop_duplicates('family_gene', inplace=True)
        
        correlating_genes_in_family = self._correlations.groupby('family_id')['family_gene'].apply(lambda x: ', '.join(x.tolist()))
        correlating_genes_in_family.name = 'correlating_genes_in_family'
        self._correlations.drop('family_gene', axis=1, inplace=True)
        self._correlations.drop_duplicates(inplace=True)
        
        self._correlations = self._correlations.join(correlating_genes_in_family, on='family_id')
        self._correlations = self._correlations.join(max_correlations, on=['family_id', 'bait_id'])
        
        # bait_families: pd.Series(index=(family : str), data=(id : int))
        bait_families = self._network.gene_families.map(self._bait_nodes)
        bait_families.dropna(inplace=True)
        bait_families.name = self._bait_nodes.name
        
        # Write it
        self.write_sif(bait_families)
        self.write_node_attr(bait_families)
        self.write_edge_attr()
        
        # Copy additional files
        for file in [get_data_file('coexpnetviz/coexpnetviz_style.xml'), get_data_file('coexpnetviz/README.txt')]:
            file.copy('.')
    
    def write_sif(self, bait_families):
        # correlation edges (without attribute data)
        correlation_edges = self._correlations.copy()
        correlation_edges['type'] = 'cor'
        correlation_edges = correlation_edges[['family_id', 'type', 'bait_id']]
        
        # homology edges
        homology_edges = pd.concat([bait_families]*2, axis=1)
        homology_edges.columns = ['id1', 'id2']
        homology_edges = homology_edges[homology_edges['id1'] >  homology_edges['id2']]  # drop self relations
        homology_edges['type'] = 'hom'
        homology_edges = homology_edges.reindex(columns='id1 type id2'.split())
        
        # sif
        bait_nodes = self._bait_nodes.to_frame(0)
        correlation_edges.columns = range(3)
        homology_edges.columns = range(3)
        
        sif = pd.concat([bait_nodes, correlation_edges, homology_edges], ignore_index=True)
        sif[[0,2]] = sif[[0,2]].applymap(self._format_node_id)
        sif.to_csv('{}.sif'.format(self._network.name), sep='\t', header=False, index=False)
    
    def write_node_attr(self, bait_families):
        ###################
        # bait node attrs
        bait_node_attrs = self._bait_nodes.reset_index()
        bait_families = bait_families.reset_index().groupby('id')['family'].agg(lambda x: ', '.join(x.tolist()))
        bait_families.name = 'families'
        bait_node_attrs = bait_node_attrs.join(bait_families, on='id')
        bait_node_attrs.rename(columns={'gene': 'bait_gene'}, inplace=True)
        bait_node_attrs['label'] = bait_node_attrs['bait_gene']
        bait_node_attrs['colour'] = '#FFFFFF'
        bait_node_attrs['type'] = 'bait node'
        bait_node_attrs['partition_id'] = hash(frozenset())
        
        ###################
        # Family node attrs
        
        # partitions: pd.Series(index=(family_id : int), data=(partition_id : int))
        partitions = self._correlations.groupby('family_id')['bait_id'].agg(lambda x: hash(frozenset(x.tolist())))
        partitions.name = 'partition_id'
        
        # colours assigned to partitions: pd.Series(index=(partition_id : int), data=(colour : str))
        colours = partitions.drop_duplicates()
        colours = pd.DataFrame(get_distinct_colours(len(colours)), index=colours)
        def to_hex(x):
            return '#' + ''.join((x*255).round().apply(lambda y: '{:02x}'.format(int(y))))
            #return ''.join(['#'] + list(map(lambda x: .format(min(255, round(x*255))), tup)))
        colours = colours.apply(to_hex, axis=1)
        colours = colours.reindex(np.random.permutation(colours.index))  # shuffle the colours so distinct colours are less likely to be put next to each other
        colours.name = 'colour'
        
        # family_colours : pd.Series(index=(family_id : int), data=(colour : str))
        family_colours = partitions.to_frame().join(colours, on='partition_id')
        
        # Family node attrs
        family_node_attrs = self._correlations[['family_id', 'family', 'correlating_genes_in_family']].set_index('family_id')
        family_node_attrs['type'] = 'family node'
        family_node_attrs['label'] = family_node_attrs['correlating_genes_in_family'] 
        family_node_attrs = family_node_attrs.join(family_colours)
        family_node_attrs.index.name = 'id'
        family_node_attrs.reset_index(inplace=True)
        
        #######################
        # concat
        nodes = pd.concat([bait_node_attrs, family_node_attrs], ignore_index=True)
        nodes = nodes.reindex(columns='id label colour type bait_gene species families family correlating_genes_in_family partition_id'.split())
        nodes['id'] = nodes['id'].apply(self._format_node_id)
        nodes.to_csv('{}.node.attr'.format(self._network.name), sep='\t', index=False)
        
    def write_edge_attr(self):
        correlation_edges = self._correlations[['family_id', 'bait_id']]
        correlation_edges = correlation_edges.apply(lambda x: '{} (cor) {}'.format(*map(self._format_node_id, x.tolist())), axis=1).to_frame('edge')
        correlation_edges['max_correlation'] = self._correlations['max_correlation']
        correlation_edges.to_csv('{}.edge.attr'.format(self._network.name), sep='\t', index=False)
        
    def _format_node_id(self, node_id):
        if np.isnan(node_id):
            return ''
        else:
            return 'n{}'.format(int(node_id))

    
    
