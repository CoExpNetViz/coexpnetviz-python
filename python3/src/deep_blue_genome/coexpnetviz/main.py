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
from deep_blue_genome.core.util import series_swap_with_index,\
    get_distinct_colours
from deep_blue_genome.core.expression_matrix import ExpressionMatrix
from deep_blue_genome.coexpnetviz.network import Network

# TODO wouldn't it be interesting to show correlation between baits as well?

'''
CoExpNetViz

Terminology used:

- bait gene: One of the genes provided by the user to which target genes are compared in terms of co-expression
- target gene: any gene that's not a bait gene
- family node: a node containing targets of the same orthology family
'''

def coexpnetviz(context, baits, gene_families, expression_matrices):
    '''
    Derive a CoExpNetViz Network.
    
    Bait node iff bait (even when no fam node correlates to it)
    
    Family nodes only contain the genes of the family that actually correlate to
    a bait. Families that don't correlate with any bait are omitted from the
    output.
    
    Partitions are the grouping of fam nodes by the subset of baits they correlate to.
    
    Parameters
    ----------
    TODO types have changed
    baits : list-like of str
        genes to which non-bait genes are compared
    gene_families : dict of gene to family
        gene families of the genes in the expression matrices. This may be omitted if all baits are of the same species
    expression_matrices : pandas.DataFrame
        gene expression matrices containing the baits and other genes
        
    Returns
    -------
    Network
        A network/graph of typed nodes, edges and partitions 
    '''
    cutoff = 0.8  # hardcoded cut-off, wooptiTODO
    
    inverted_gene_families = gene_families.reset_index().set_index('gene')
    
    # Correlations
    # Note: for genes not part of any family we assume they are part of some family, just not one of the ones provided. (so some family nodes have None as family)
    correlations = []
    for exp_mat in expression_matrices:
        matrix = exp_mat.data
        
        # Baits present in matrix
        baits_mask = matrix.index.isin(baits)
        baits_ = matrix.index[baits_mask]
        
        # Correlation matrix
        corrs = ExpressionMatrix(matrix).pearson_r(baits_)
        corrs.to_csv(exp_mat.name + '.corr_mat.txt', sep='\t', na_rep=str(np.nan))
        
        # correlations
        corrs.columns = baits_
        corrs.drop(baits_, inplace=True)
        corrs = corrs[abs(corrs) > cutoff]
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
    parser.add_argument('--baits-file', metavar='B', required=True,
                       help='path to file listing the bait genes to use')
    parser.add_argument('--gene-families', metavar='F',
                       help='path to file with gene families to use. If omitted, Plaza is used')
    parser.add_argument('-e', '--expression-matrices', metavar='M', required=True, nargs='+',
                       help='path to expression matrix to use')
    args = parser.parse_args(argv)

    # Read files
    baits = read_baits_file(args.baits_file)
    gene_families = read_gene_families_file(args.gene_families) if args.gene_families else None
    expression_matrices = [read_expression_matrix_file(matrix) for matrix in args.expression_matrices] # TODO should remove non-varying rows in user-submitted exp mats (in any exp mats really, but we can trust our own prepped mats already have this step performed)
    
    # Run alg
    context = None # TODO
    network = coexpnetviz(context, baits, gene_families, expression_matrices)

    # Write network to cytoscape files
    CytoscapeWriter(network).write()
    
# TODO consider one of the formats that allows specifying the network, edges with attributes and nodes with attributes all at once. Perhaps even with a style.
# e.g. XGMLL from http://wiki.cytoscape.org/Cytoscape_3/UserManual#Cytoscape_3.2BAC8-UserManual.2BAC8-Network_Formats.XGMML_Format
class CytoscapeWriter(object):
    def __init__(self, network):
        self._network = network
        
    def write(self):
        # assign node ids to baits
        self._bait_nodes = self._network.baits.copy()
        self._bait_nodes.index.name = 'id'
        self._bait_nodes = series_swap_with_index(self._bait_nodes)
        
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
        self._correlations.set_index('bait', inplace=True)
        self._correlations = self._correlations.join(self._bait_nodes)
        self._correlations.reset_index(inplace=True)
        self._correlations.rename(columns={'index': 'bait', 'id': 'bait_id'}, inplace=True)
        
        # bait_families: pd.Series(index=(family : str), columns=[(id : int)])
        bait_families = self._network.gene_families.to_frame().join(self._bait_nodes, on='gene', how='right')
        bait_families.drop('gene', axis=1, inplace=True)
        
        # Write it
        self.write_sif(bait_families)
        self.write_node_attr(bait_families)
        self.write_edge_attr()
    
    def write_sif(self, bait_families):
        # correlation edges (without attribute data)
        correlation_edges = self._correlations.copy()
        correlation_edges['type'] = 'cor'
        correlation_edges = correlation_edges[['family_id', 'type', 'bait_id']]
        
        # homology edges
        homology_edges = bait_families.join(bait_families, lsuffix='1', rsuffix='2')
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
        
        ###################
        # Family node attrs
        
        # partitions: pd.Series((partition_id : int), index=(family_id : int))
        partitions = self._correlations.groupby('family_id')['bait_id'].agg(lambda x: hash(frozenset(x.tolist())))
        partitions.name = 'partition_id'
        
        # colours assigned to partitions: pd.Series((colour : str), index=(partition_id : int))
        colours = partitions.drop_duplicates()
        colours = pd.Series(get_distinct_colours(len(colours)), index=colours) 
        def to_hex(tup):
            return ''.join(['#'] + list(map(lambda x: '{:02x}'.format(min(255, round(x*255))), tup)))
        colours = colours.apply(to_hex)
        colours.name = 'colour'
        
        # family_colours : pd.Series((colour : str), index=(family_id : int))
        family_colours = partitions.to_frame().join(colours, on='partition_id')['colour']
        del partitions
        del colours
        
        # Family node attrs
        family_node_attrs = self._correlations[['family_id', 'family']].set_index('family_id')
        family_node_attrs['label'] = self._correlations.groupby('family_id')['family_gene'].apply(lambda x: ', '.join(x.tolist()))
        family_node_attrs['type'] = 'family node'
        family_node_attrs['correlating_genes_in_family'] = family_node_attrs['label']
        family_node_attrs = family_node_attrs.join(family_colours)
        family_node_attrs.index.name = 'id'
        family_node_attrs.reset_index(inplace=True)
        
        #######################
        # concat
        nodes = pd.concat([bait_node_attrs, family_node_attrs], ignore_index=True)
        nodes = nodes.reindex(columns='id label colour type bait_gene species families family correlating_genes_in_family'.split())
        nodes['id'] = nodes['id'].apply(self._format_node_id)
        nodes.to_csv('{}.node.attr'.format(self._network.name), sep='\t', index=False)
        
    def write_edge_attr(self):
        correlation_edges = self._correlations[['family_id', 'bait_id']]
        correlation_edges = correlation_edges.apply(lambda x: '{} (cor) {}'.format(*map(self._format_node_id, x.tolist())), axis=1).to_frame('edge')
        correlation_edges['r_value'] = self._correlations['correlation']
        correlation_edges.to_csv('{}.edge.attr'.format(self._network.name), sep='\t', index=False)
        
    def _format_node_id(self, node_id):
        if np.isnan(node_id):
            return ''
        else:
            return 'n{}'.format(int(node_id))

if __name__ == '__main__':
    main()
    
    
    
