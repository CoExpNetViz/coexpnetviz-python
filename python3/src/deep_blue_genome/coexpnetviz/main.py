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
import pandas
from deep_blue_genome.core.util import invert_multidict,\
    keydefaultdict, invert_dict
from collections import defaultdict, namedtuple
from deep_blue_genome.core.expression_matrix import ExpressionMatrix
from functools import reduce
from deep_blue_genome.coexpnetviz.node import BaitNode, FamilyNode

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

Network = namedtuple('Network', 'name bait_nodes family_nodes homology_edges partitions gene_to_families gene_families'.split())

def coexpnetviz(context, baits, gene_families, expression_matrices):
    '''
    Run CoExpNetViz.
    
    Calculate the following:
    
    - bait nodes
    - family nodes
    - partitions
    
    Bait node iff bait (even when no fam node correlates to it)
    
    Family nodes only contain the genes of the family that actually correlate to
    a bait. Families that don't correlate with any bait are omitted from the
    output.
    
    Partitions are the grouping of fam nodes by the subset of baits they correlate to.
    
    Parameters
    ----------
    baits : list-like of str
        genes to which non-bait genes are compared
    gene_families : dict of gene to family
        gene families of the genes in the expression matrices. This may be omitted if all baits are of the same species
    expression_matrices : pandas.DataFrame
        gene expression matrices containing the baits and other genes
        
    Returns
    -------
    (nodes, edges, partitions)
        A network/graph of typed nodes, edges and partitions 
    '''
    cutoff = 0.8  # hardcoded cut-off, wooptiTODO
    
    # Create bait nodes
    bait_nodes = pandas.Series({bait: BaitNode(bait) for bait in baits})
    
    # Add singleton families for genes without family
    # Note: if this is too slow, just don't create them up front as we don't need most of them 
    orphans = reduce(lambda x,y: x.union(y), (exp_mat.data.index for exp_mat in expression_matrices))
    orphans.drop(set.union(*map(set, gene_families.values())), errors='ignore')
    for i, orphan in enumerate(orphans):
        gene_families['singleton{}'.format(i)] = {orphan}
    
    gene_to_families = invert_multidict(gene_families)
    
    # Create family nodes, correlation edges
    family_nodes = keydefaultdict(lambda name: FamilyNode(name))  # family name -> FamilyNode
    inverse_partitions = defaultdict(lambda: set())
    for exp_mat in expression_matrices:
        matrix = exp_mat.data
        
        # Baits present in matrix
        baits_mask = matrix.index.isin(baits)
        baits_ = matrix.index[baits_mask]
        
        # Correlation matrix
        corrs = ExpressionMatrix(matrix).pearson_r(baits_)
        corrs.to_csv(exp_mat.name + '.corr_mat.txt', sep='\t')
        
        # Family nodes, CorrelationEdges
        corrs.columns = bait_nodes[baits_]
        corrs.drop(baits_, inplace=True)
        corrs = corrs[abs(corrs) > cutoff].dropna(how='all')
        for (target, bait_node), correlation in corrs.stack().to_dict().items():
            family_nodes_ = (family_nodes[name] for name in gene_to_families[target])
            for family_node in family_nodes_:
                family_node.add_correlation(bait_node, correlation, family_gene=target)
                inverse_partitions[family_node].add(bait_node)
                
    # Create partitions
    inverse_partitions = {k: frozenset(v) for k,v in inverse_partitions.items()}
    partitions = pandas.DataFrame.from_dict(invert_dict(inverse_partitions)) #TODO how would have created as dataframe from the start?
    #TODO should load multi stuff as tables or as dict -> []?  # Got a performance case to think about?
    # TODO dataframes can have `name`, so maybe we can get rid of ExpressionMatrix class again
                
    # Add homology edges between baits (some genes in a family node will surely also be in another family node, but we won't show that)
    data_frames = []
    for i, bait in enumerate(baits):
        if bait not in gene_to_families:
            continue
        genes = set.union(*(gene_families[family_name] for family_name in gene_to_families[bait])) 
        data_frames.append(pandas.DataFrame([bait_nodes[bait], bait_nodes[bait_]] for bait_ in set(baits[i+1:]) & genes))
    homology_edges = pandas.concat(data_frames, ignore_index=True)
    homology_edges.columns = 'left right'.split()
    
    # Return
    return Network(
        name='network',
        bait_nodes=bait_nodes.tolist(),
        family_nodes=list(family_nodes.values()),
        homology_edges=homology_edges,
        partitions=partitions,
        gene_to_families=gene_to_families,
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
    baits = list({bait.lower() for bait in read_baits_file(args.baits_file)})
    gene_families = read_gene_families_file(args.gene_families) if args.gene_families else None
    expression_matrices = [read_expression_matrix_file(matrix) for matrix in args.expression_matrices] # TODO should remove non-varying rows in user-submitted exp mats (in any exp mats really, but we can trust our own prepped mats already have this step performed)
    
    # Run alg
    context = None # TODO
    network = coexpnetviz(context, baits, gene_families, expression_matrices)

    # Write network to cytoscape files
    CytoscapeWriter(network).write()
    assert False
    
class CytoscapeWriter(object):
    def __init__(self, network):
        self._network = network
        
    def write_cytoscape(self):
        self.write_sif()
        self.write_node_attr()
        self.write_edge_attr()
    
    def write_sif(self):
        #sif = ?
        sif.to_csv('{}.sif'.format(self._network.name), sep='\t')
    
    def write_node_attr(self):
        bait_nodes = pandas.DataFrame(
            [id(node), node.name, node.name, np.nan, 
                ', '.join(self._networkgene_to_families[node.name])]
                for node in self._network.bait_nodes,
            columns='id label bait_gene species families'.split()
        )
        bait_nodes['colour'] = '#FFFFFF'
        bait_nodes['type'] = 'bait node'
        
        self._network.partitions
        
        family_nodes = pandas.DataFrame(
            [id(node), ', '.join(node.genes), node.name]
                for node in self._network.family_nodes,
            columns='id label family'.split()
        )
        family_nodes['colour'] = part color
        family_nodes['type'] = 'family node'
        family_nodes['correlating_genes_in_family'] = family_nodes['label']
        
        nodes = pandas.concat([bait_nodes, family_nodes], ignore_index=True)
        nodes = nodes.reindex(columns='id label colour type bait_gene species families'.split())
        nodes.fillna('')
        nodes.to_csv('{}.node.attr'.format(self._network.name), sep='\t')
        
    def write_edge_attr(self):
        homology_edges = self._network.homology_edges.copy()
        homology_edges['type'] = 'hom'
        correlation_edges = pandas.concat(node.correlation_edges for node in self._network.family_nodes, ignore_index=True)
        correlation_edges['type'] = 'cor'
        edges = p.concat([homology_edges, correlation_edges], ignore_index=True)
        edges = edges.reindex(columns='left type right value'.split())
        cols = ['left','right']
        edges[cols] = edges[cols].applymap(id)
        edges.fillna('')
        edges.to_csv('{}.edge.attr'.format(self._network.name), sep='\t')

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
    
    
    
