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

# TODO wouldn't it be interesting to show correlation between baits as well?

from deep_blue_genome.core.reader.various import read_baits_file, read_expression_matrix_file,\
    read_gene_families_file
import sys
import argparse
import pandas as pd
import numpy as np
from deep_blue_genome.core.util import keydefaultdict, invert_dict
from collections import defaultdict
from deep_blue_genome.core.expression_matrix import ExpressionMatrix
from deep_blue_genome.coexpnetviz.node import BaitNode, FamilyNode
from deep_blue_genome.coexpnetviz.network import Network

# TODO better partition colors
# - http://stackoverflow.com/a/30881059/1031434
# - http://stackoverflow.com/a/4382138/1031434
# - http://phrogz.net/css/distinct-colors.html

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
    
    # Create bait nodes
    bait_nodes = baits.apply(BaitNode)
    bait_nodes.index = baits
    
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
        corrs.to_csv(exp_mat.name + '.corr_mat.txt', sep='\t')
        
        # correlations
        corrs.columns = bait_nodes[baits_]
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
    
    # Family nodes
    family_nodes = correlations['family'].drop_duplicates()  # not yet family nodes, but it will be
    family_nodes.dropna(inplace=True)
    family_nodes.index = family_nodes
    family_nodes = family_nodes.apply(FamilyNode)
    family_nodes.name = 'family_node'
    
    # correlations (continued)
    correlations.set_index('family', inplace=True)
    correlations = correlations.join(family_nodes)
    correlations.reset_index(drop=True, inplace=True)
    correlations.rename(columns={'family_node' : 'family'}, inplace=True)
    correlations = correlations.reindex(columns='family family_gene bait correlation'.split())
    
    # Return
    return Network(
        name='network',
        bait_nodes=bait_nodes.tolist(),
        family_nodes=list(family_nodes.values()),
        gene_families=gene_families
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
    assert False
    
# TODO consider one of the formats that allows specifying the network, edges with attributes and nodes with attributes all at once. Perhaps even with a style.
# e.g. XGMLL from http://wiki.cytoscape.org/Cytoscape_3/UserManual#Cytoscape_3.2BAC8-UserManual.2BAC8-Network_Formats.XGMML_Format
class CytoscapeWriter(object):
    def __init__(self, network):
        self._network = network
        
    def write_cytoscape(self):
        self.write_sif()
        self.write_node_attr()
        self.write_edge_attr()
    
    def write_sif(self):
        #pd.concat
        # list all nodes and relations
        self._network.baits
        
        #
        self._network.targets
        
        # correlation edges (without attribute data)
        self._network.correlations.drop('', axis=1)
        
        # homology edges
        self._network.gene_families[]
        
        #sif = ?
        #multiple lines allowed
        
        homology_edges = self._network.homology_edges.copy()
        homology_edges['type'] = 'hom'
        sif.to_csv('{}.sif'.format(self._network.name), sep='\t') # TODO no header, no () around relations
    
    def write_node_attr(self):
        # TODO header in attr files
        bait_nodes = pd.DataFrame(
            ([id(node), node.name, node.name, np.nan, 
                ', '.join(self._networkgene_to_families[node.name])]
                for node in self._network.bait_nodes),
            columns='id label bait_gene species families'.split()
        )
        bait_nodes['colour'] = '#FFFFFF'
        bait_nodes['type'] = 'bait node'
        
        self._network.partitions
        
        family_nodes = pd.DataFrame(
            ([id(node), ', '.join(node.genes), node.name]
                for node in self._network.family_nodes),
            columns='id label family'.split()
        )
#         TODO family_nodes['colour'] = part color
        family_nodes['type'] = 'family node'
        family_nodes['correlating_genes_in_family'] = family_nodes['label']
        
        nodes = pd.concat([bait_nodes, family_nodes], ignore_index=True)
        nodes = nodes.reindex(columns='id label colour type bait_gene species families'.split())
        nodes.fillna('')
        nodes.to_csv('{}.node_attr.txt'.format(self._network.name), sep='\t')
        
    def write_edge_attr(self):
        # TODO header in attr files
        # TODO no hom needed here
        # TODO () around relations
        correlation_edges = pd.concat((node.correlation_edges for node in self._network.family_nodes), ignore_index=True)
        correlation_edges['type'] = 'cor'
        edges = p.concat([homology_edges, correlation_edges], ignore_index=True)
        edges = edges.reindex(columns='left type right value'.split())
        cols = ['left','right']
        edges[cols] = edges[cols].applymap(id)
        edges.fillna('')
        edges.to_csv('{}.edge.attr'.format(self._network.name), sep='\t')
        
        
        assert False
        
        # Create partitions
        partitions.append((family_node, bait_node))
        partitions = pd.DataFrame(partitions, columns='family bait'.split())
        partitions.drop_duplicates(inplace=True)
        print(partitions.head())
                    
        # Add homology edges between baits (some genes in a family node will surely also be in another family node, but we won't show that)
        #TODO don't need this here, am using it elsewhere, but not here
        data_frames = []
        for i, bait in enumerate(baits):
            if bait not in gene_to_families:
                continue
            genes = set.union(*(gene_families[family_name] for family_name in gene_to_families[bait])) 
            data_frames.append(pd.DataFrame([bait_nodes[bait], bait_nodes[bait_]] for bait_ in set(baits[i+1:]) & genes))
        homology_edges = pd.concat(data_frames, ignore_index=True)
        homology_edges.columns = 'left right'.split()

#     
#     
#     /**
#      * Write same node relations as sif, but in more detail
#      */
#     void CytoscapeWriter::write_edge_attr() {
#         ofstream out(network_name + ".edge.attr");
#         out.exceptions(ofstream::failbit | ofstream::badbit);
#         out << "edge\tr_value\n";
#     
#         // target -> bait correlation
#         for (auto&& neigh : neighbours) {
#             if (!neigh->get_bait_correlations().empty()) {
#                 for (auto& bait_correlation : neigh->get_bait_correlations()) {
#                     out << target_nodes[neigh].get_cytoscape_id() << " (cor) " << bait_nodes[&bait_correlation.get_bait()].get_cytoscape_id() << "\t" << bait_correlation.get_max_correlation() << "\n";
#                 }
#             }
#         }
#     
#         // bait <-> bait orthology
#         for (auto& p : get_bait_orthology_relations()) {
#             out << bait_nodes[p.first].get_cytoscape_id() << " (hom) " << bait_nodes[p.second].get_cytoscape_id() << "\tNA\n";
#         }
#     }
#
#     /**
#      * Get orthology relations between baits
#      */
#     std::vector<BaitBaitOrthRelation>& CytoscapeWriter::get_bait_orthology_relations() {
#         if (!bait_orthologies_cached) {
#             // get a set of ortholog groups of the baits
#             flat_set<const OrthologGroupInfo*> bait_groups;
#             for (auto bait : baits) {
#                 boost::insert(bait_groups, groups.get(*bait) | referenced);
#             }
#     
#             // enumerate all possible pairs of baits of the same group
#             for (auto group : bait_groups) {
#                 // filter out genes not present in our set of baits
#                 vector<const Gene*> genes;
#                 assert(group);
#                 for (auto gene : group->get().get_genes()) {
#                     if (contains(baits, gene)) {
#                         genes.emplace_back(gene);
#                     }
#                 }
#     
#                 // output all pairs
#                 for (auto it = genes.begin(); it != genes.end(); it++) {
#                     for (auto it2 = it+1; it2 != genes.end(); it2++) {
#                         bait_orthologies.emplace_back(make_pair(*it, *it2));
#                         bait_orthologies.emplace_back(make_pair(*it2, *it));
#                     }
#                 }
#             }
#             erase_duplicates(bait_orthologies);
#             bait_orthologies_cached = true;
#         }
#     
#         return bait_orthologies;
#     }

# TODO could merging of ortho groups be done badly? Look at DataFileImport
# TODO no mention of gene variants (unless as a warning to when we encounter a gene variant)

# for morph:
# TODO a meta-db that tells us what exp mats we have, e.g. search by the species it contains

if __name__ == '__main__':
    main()
    
    
    
