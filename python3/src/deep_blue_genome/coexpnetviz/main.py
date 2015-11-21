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
from deep_blue_genome.core.util import print_abbreviated_dict, invert_multidict,\
    keydefaultdict, invert_dict
from collections import defaultdict, namedtuple
from deep_blue_genome.core.expression_matrix import ExpressionMatrix

'''
CoExpNetViz

Terminology used:

- bait gene: One of the genes provided by the user to which target genes are compared in terms of co-expression
- target gene: any gene that's not a bait gene
- family node: a node containing targets of the same orthology family
'''

Network = namedtuple('Network', 'bait_nodes family_nodes edges partitions'.split())
    
class Node(object):
    
    def __init__(self, name):
        self._name = name
        
    @property
    def name(self):
        return self._name
    
    def __lt__(self, other):
        return self.name < other.name
    
class BaitNode(Node):
    
    def __init__(self, name):
        super().__init__(name)
    
    def __repr__(self):
        return 'BaitNode({!r})'.format(self.name)

class FamilyNode(Node):
    
    def __init__(self, family_name):
        super().__init__(family_name)
    
    def __repr__(self):
        return 'FamilyNode({!r})'.format(self.name)
    
class CorrelationEdge(object):
    
    def __init__(self, left, right, correlation):
        self._left = left
        self._right = right
        self._correlation = correlation

class HomologyEdge(object):
    
    def __init__(self, left, right):
        self._left = left
        self._right = right

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
    baits : list of gene
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
    
    gene_to_families = invert_multidict(gene_families)
    
    # Create family nodes, correlation edges
    family_nodes = keydefaultdict(lambda name: FamilyNode(name))  # family name -> FamilyNode
    edges = []
    inverse_partitions = defaultdict(lambda: set())
    for exp_mat in expression_matrices:
        matrix = exp_mat.data
        
        # Baits present in matrix
        baits_mask = matrix.index.isin(baits)
        baits_ = matrix.index[baits_mask]
        
        # Drop genes which aren't part of a family (and thus can't be used for comparative transcriptomics)
        # TODO Log genes not part of a family
        matrix = matrix.loc[matrix.index.isin(gene_to_families.keys()) | baits_mask]
        
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
                edges.append(CorrelationEdge(family_node, bait_node, correlation))
                inverse_partitions[family_node].add(bait_node)
                
    # Create partitions
    inverse_partitions = {k: frozenset(v) for k,v in inverse_partitions.items()}
    partitions = invert_dict(inverse_partitions)
                
    # Add homology edges between baits (some genes in a family node will surely also be in another family node, but we won't show that)
    for i, bait in enumerate(baits):
        if bait not in gene_to_families:
            continue
        genes = set.union(*(gene_families[family_name] for family_name in gene_to_families[bait])) 
        edges.extend(HomologyEdge(bait_nodes[bait], bait_nodes[bait_]) for bait_ in set(baits[i+1:]) & genes)
    
    # Return
    return Network(
        bait_nodes = bait_nodes.tolist(),
        family_nodes = list(family_nodes.values()),
        edges=edges,
        partitions=partitions
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
    baits = [bait.lower() for bait in read_baits_file(args.baits_file)]
    gene_families = read_gene_families_file(args.gene_families) if args.gene_families else None
    expression_matrices = [read_expression_matrix_file(matrix) for matrix in args.expression_matrices] # TODO should remove non-varying rows in user-submitted exp mats (in any exp mats really, but we can trust our own prepped mats already have this step performed)
    
    # Run alg
    context = None # TODO
    graph = coexpnetviz(context, baits, gene_families, expression_matrices)

    # Write network to cytoscape files
    print('TODO write') # TODO
    

#     unique_ptr<OrthologGroupInfos> groups;

    # Write output
#     CytoscapeWriter writer(install_dir, baits, neighbours, *groups);
#     writer.write();

# void read_yaml(std::string path, Database& database, string& baits_path, double& negative_treshold, double& positive_treshold, unique_ptr<OrthologGroupInfos>& groups, vector<GeneExpressionMatrix*>& expression_matrices) {
#     YAML::Node job_node = YAML::LoadFile(path);
#     baits_path = job_node["baits"].as<string>();
# 
#     negative_treshold = job_node["negative_treshold"].as<double>();
#     ensure(fabs(negative_treshold) <= 1.0+1e-7, "negative_treshold must be a double between -1 and 1", ErrorType::GENERIC);
# 
#     positive_treshold = job_node["positive_treshold"].as<double>();
#     ensure(fabs(positive_treshold) <= 1.0+1e-7, "positive_treshold must be a double between -1 and 1", ErrorType::GENERIC);
# 
#     DataFileImport importer(database);
# 
#     for (auto matrix_node : job_node["expression_matrices"]) {
#         string matrix_path = matrix_node.as<string>();
#         string matrix_name = matrix_path;
#         auto& matrix = importer.add_gene_expression_matrix(matrix_name, matrix_path);
# 
#         expression_matrices.emplace_back(&matrix);
#     }
# 
#     auto&& orthologs_node = job_node["orthologs"];
#     if (orthologs_node.size() > 0) {
#         // clear current list of homology families
#         database.erase_families();
# 
#         // add new families
#         read_orthologs_yaml(database, orthologs_node); // TODO in general you want every command to take something of the form that acts like database --add before running the alg; to allow the user to add his/her own data.
#     }
# 
#     // Get set of all genes
#     unordered_set<Gene*> genes;
#     for (auto& matrix : expression_matrices) {
#         for (auto& gene : matrix->get_genes()) {
#             ensure(genes.emplace(gene).second,
#                     (make_string() << "Gene " << *gene << " present in multiple matrices").str()
#             );;
#         }
#     }
# 
#     // Make sure each gene is part of at least 1 family, even if it's just a singleton
#     size_t singleton_id = 0;
#     for (auto&& gene : genes) {
#         if (boost::empty(gene->get_ortholog_groups())) {
#             database.add_ortholog_group(GeneFamilyId("singleton", (make_string() << singleton_id++).str())).add(*gene);
#         }
#     }
# 
#     //
#     groups = make_unique<OrthologGroupInfos>();
# }
# 
# /**
#  * Load baits as distinct list of groups
#  */
# std::vector<Gene*> load_baits(Database& database, std::string baits_path) {
#     vector<Gene*> baits;
# 
#     // read file
#     Baits baits_(baits_path);
#     for (const auto& gene_name : baits_.get_genes()) {
#         auto& gene = database.get_gene(gene_name);
#         baits.emplace_back(&gene);
#     }
# 
#     // get rid of duplicates in input
#     sort(baits.begin(), baits.end());
#     unique(baits.begin(), baits.end());
# 
#     return baits;
# }

# TODO could merging of ortho groups be done badly? Look at DataFileImport
# TODO no mention of gene variants (unless as a warning to when we encounter a gene variant)

# for morph:
# TODO a meta-db that tells us what exp mats we have, e.g. search by the species it contains

if __name__ == '__main__':
    main()
    
    
    
