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
from deep_blue_genome.reader import read_baits_file, read_expression_matrix_file,\
    read_gene_families_file
import sys
import argparse
from deep_blue_genome.context import Context

# TODO move to init vvv
'''
CoExpNetViz

Terminology used:

- bait gene: One of the genes provided by the user to which target genes are compared in terms of co-expression
- target gene: any gene that's not a bait gene
- family node: a node containing targets of the same orthology family
'''
 
def coexpnetviz(context, baits, gene_families, expression_matrices):
    '''
    Run CoExpNetViz.
    
    Parameters
    ----------
    baits : list of gene
        genes to which non-bait genes are compared
    gene_families : dict of gene to family
        gene families of the genes in the expression matrices. This may be omitted if all baits are of the same species
    expression_matrices : pandas.DataFrame
        gene expression matrices containing the baits and other genes 
    '''
    #print(baits)
    #print(gene_families)
    #print(expression_matrices)
    
    for expression_matrix in expression_matrices:
        for name in expression_matrix.index:
            # TODO ping pong name finding is waaaay too slow. Batch name find might do. But if the data files are small (and thus quick to load in memory; or we can grab a local database; then that shit is better. Want also to remain easy to configure on some random biologist's pc)
            print(name)
            context.genes.find_by_name(name)
    cutoff = 0.8  # hardcoded cut-off, wooptido
    
    # Simply run it


# Why get the geneid? Some genes have multiple names, with mere strings they wouldn't match, after mapping them to their id, they would.

# We don't actually care what organism (or taxon rather) a gene is of. We just want all the matrices that have in it, the genes that we are looking for 

# TODO compute
# TODO write cytoscape
# ignore species of genes for now, genes will be some kind of object; and they will have info on them, but not yet
# TODO try build merged plaza in python. Add a test

# We need to abstract our actual data sources (e.g. Entrez genes, vs NCBI genes, ...)
# TODO something to identify a gene from its name and get its species name (take into account that some gene names aren't globally unique (the horrors)) 
# Consider storing data according to BioSQL standard using BioPython modules. This way all Bio* can work on the same data at the same time; because database + shared schema. Screw performance, that's only for when the hardware becomes too expensive to calc it.

# TODO no yaml, just a python func that can be called directly to do all of it
# TODO just document for sphinx in numpydoc syntax, and http://stackoverflow.com/a/24385103/1031434
# TODO no mention of gene variants (unless as a warning to when we encounter a gene variant)

# for morph:
# TODO a meta-db that tells us what exp mats we have, e.g. search by the species it contains

if __name__ == '__main__':
    # Parse CLI args
    parser = argparse.ArgumentParser(description='Comparative Co-Expression Network Construction and Visualization (CoExpNetViz): Command line interface')
    parser.add_argument('-b', '--baits-file', metavar='B', required=True,
                       help='path to file listing the bait genes to use')
    parser.add_argument('-f', '--gene-families', metavar='F',
                       help='path to file with gene families to use. If omitted, Plaza is used')
    parser.add_argument('-e', '--expression-matrices', metavar='M', required=True, nargs='+',
                       help='path to expression matrix to use')
    args = parser.parse_args()

    # Read files
    baits = read_baits_file(args.baits_file)
    gene_families = read_gene_families_file(args.gene_families) if args.gene_families else None
    expression_matrices = [read_expression_matrix_file(matrix) for matrix in args.expression_matrices]
    
    # Run alg
    context = Context()
    coexpnetviz(context, baits, gene_families, expression_matrices)
    
    # Write
    print('TODO write') # TODO

#     string install_dir = fs::canonical(fs::path(argv[0])).remove_filename().parent_path().native();

    # Load database
#     Database database(argv[1]);

    # Read input
#     string baits_path;
#     unique_ptr<OrthologGroupInfos> groups;
#     vector<GeneExpressionMatrix*> expression_matrices;
#     read_yaml(argv[2], database, baits_path, negative_treshold, positive_treshold, groups, expression_matrices);
# 
#     vector<Gene*> baits = load_baits(database, baits_path);
# 
#     cout.flush();

    # Actual computation:
    #
    # Grab union of neighbours of each bait, where neighbour relation is sufficient (anti-)correlation
    #
    # Put differently, filter the coexpression network by: link contains a bait, abs(correlation) is sufficiently high.
    # Additionally, group non-bait genes by gene family
#     std::vector<OrthologGroupInfo*> neighbours;
#     for (auto expression_matrix : expression_matrices) {
#         # Filter baits and transform them to row indices
#         flat_set<GeneExpressionMatrixRow> indices;
#         boost::insert(indices, baits
#                 | indirected
#                 | boost::adaptors::filtered(std::bind(&GeneExpressionMatrix::has_gene, expression_matrix, _1))
#                 | transformed(std::bind(&GeneExpressionMatrix::get_gene_row, expression_matrix, _1))
#         );
# 
#         # Calculate correlations
#         GeneCorrelationMatrix correlations(*expression_matrix, indices);
#         auto& correlations_ = correlations.get();
# 
#         # Make edges to nodes with sufficient correlation
#         for (auto row_index : indices) {
#             auto col_index = correlations.get_column_index(row_index);
#             auto& bait = expression_matrix->get_gene(row_index);
#             for (GeneExpressionMatrixRow row = 0; row < correlations_.size1(); row++) {
#                 if (contains(indices, row)) {
#                     continue; // Don't make edges between baits
#                 }
# 
#                 auto corr = correlations_(row, col_index);
#                 if (corr < negative_treshold || corr > positive_treshold) {
#                     auto& gene = expression_matrix->get_gene(row);
#                     for (auto&& group : groups->get(gene)) {
#                         group.add_bait_correlation(gene, bait, corr);
#                         neighbours.emplace_back(&group);
#                     }
#                 }
#             }
# 
#             # Output correlation matrix
#             boost::filesystem::path path(expression_matrix->get_name());  // XXX this part is coupled to the way we name the matrices while adding them
#             std::ofstream out(path.filename().native() + ".correlation_matrix"); // TODO write_file util func that opens an ofstream in this proper way for us
#             out.exceptions(ofstream::failbit | ofstream::badbit);
#             out << DEEP_BLUE_GENOME::COMMON::WRITER::write_plain(*expression_matrix, correlations);
#         }
#     }
# 
#     sort(neighbours.begin(), neighbours.end());
#     neighbours.erase(unique(neighbours.begin(), neighbours.end()), neighbours.end());

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