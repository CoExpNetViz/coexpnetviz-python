// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
#include <deep_blue_genome/common/GeneMappingId.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <yaml-cpp/yaml.h>

using namespace std;

class GeneExpressionMatrixId
{
public:
	GeneExpressionMatrixId(std::string species, std::string name)
	:	species(species), name(name)
	{
	}

	std::string species; // TODO priv
	std::string name;
};

class Group
{
public:
	/**
	 * @param genes Iterable of genes
	 */
	template<IterableT>
	Group(IterableT genes)
	:	genes(genes)
	{
	}

private:
	const vector<std::string> genes; // ordered
};

int main(int argc, char** argv) {
	using namespace DEEP_BLUE_GENOME;
	using namespace DEEP_BLUE_GENOME::COEXPR;
	graceful_main([argc, argv](){
		// Read args
		if (argc != 2) {
			cout
				<< "Usage: coexpr yaml_file\n"
				<< "\n"
				<< "- yaml_file: path to file in yaml format with description of what to calculate\n"
				<< endl;

			ensure(false, "Invalid argument count", ErrorType::GENERIC);
		}

		// Load database
		Database database("/home/limyreth/dbg_db"); // TODO not hardcoded

		// Read yaml
		YAML::Node job_node = YAML::LoadFile(argv[1]);
		string baits_path = job_node["baits"].as<string>();

		double negative_treshold = job_node["negative_treshold"].as<double>();
		ensure(fabs(negative_treshold) <= 1.0+1e-7, "negative_treshold must be a double between -1 and 1", ErrorType::GENERIC);

		double positive_treshold = job_node["positive_treshold"].as<double>();
		ensure(fabs(positive_treshold) <= 1.0+1e-7, "positive_treshold must be a double between -1 and 1", ErrorType::GENERIC);

		unordered_set<std::string> species_list;
		vector<GeneExpressionMatrixId> expression_matrices;
		for (auto matrix_node : job_node["expression_matrices"]) {
			string species = matrix_node["species"].as<string>();
			string matrix_name = matrix_node["name"].as<string>();

			bool created = species_list.emplace(species).second;
			ensure(created, "Specified more than 1 matrix for the same species", ErrorType::GENERIC);

			expression_matrices.emplace_back(species, matrix_name);
		}

		// Load ortholog mappings
		map<OrthologMappingId, shared_ptr<GeneMapping>> ortholog_mappings; // TODO db update needs to make inverse mappings too. TODO we must grab from plaza...
		for (auto& source_species : species_list) {
			for (auto& target_species : species_list) {
				ortholog_mappings.emplace(piecewise_construct, forward_as_tuple(species, target_species), forward_as_tuple(database.get_ortholog_mapping(source_species, target_species)));
			}
		}

		// Load baits
		map<std::string, vector<string>> baits; // species -> baits without their orthologs
		map<std::string, vector<std::string>> orthologs; // bait -> bait's orthologs including itself
		{
			vector<string> all_baits; // kept to make sure no bait is taken into account twice
			Baits baits_(baits_path);

			// Get baits and their orthologs
			for (auto gene : baits_.get_genes()) {
				if (contains(all_baits, gene)) {
					continue;
				}

				auto species = database.get_species_of_gene(gene);
				baits[species].emplace(gene);

				auto& baits = baits[name];
				baits.emplace(gene);
				for (auto& name : species_list) {
					auto mapping = ortholog_mappings[name];
					for (auto& g : mapping->get(gene)) {
						baits.emplace(g);
					}
				}

				for (auto& g : baits) {
					all_baits.emplace(g);
				}
			}
		}

		for (auto& expression_matrix_id : expression_matrices) {
			auto gene_expressions = database.get_gene_expression_matrix(expression_matrix_id.species, expression_matrix_id.name);

			auto& baits = baits[gene_expressions->get_species_name()];
		}

		// Get baits as indices
		std::vector<size_type> baits_;
		for (auto gene : baits_.get_genes()) {
			baits_.emplace_back(gene_expressions->get_gene_index(gene));
		}

		// find neighbours of each bait
		std::unordered_map<size_type, std::vector<size_type>> bait_neighbours; // bait row index -> genes correlated above pos treshold or below neg treshold
		{
			GeneCorrelationMatrix correlations(*gene_expressions, baits_);
			{ // TODO rm debug output
				auto& correlations_ = correlations.get();

				cout << "\t";
				for (auto bait : baits_) {
					cout << gene_expressions->get_gene_name(bait) << "\t";

				}
				cout << "\n";

				for (size_type gene=0; gene < correlations_.size1(); gene++) {
					cout << gene_expressions->get_gene_name(gene) << "\t";
					for (auto bait : baits_) {
						cout << correlations_(gene, bait) << "\t";
					}
					cout << "\n";
				}
			}

			for (auto bait : baits_) {
				auto& neighbours = bait_neighbours.emplace(piecewise_construct, forward_as_tuple(bait), forward_as_tuple()).first->second;
				auto& correlations_ = correlations.get();
				auto col_index = correlations.get_column_index(bait);

				for (size_type gene = 0; gene < correlations_.size1() && gene != bait; gene++) {
					auto corr = correlations_(gene, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						neighbours.emplace_back(gene);
					}
				}
			}
		}

		////////////////////////////
		// Take intersections

		// Efficiently grabbing any group (=gene grouped with all its orthologs):
		// - grab with ortholog mapping
		// - wrap in a Group object
		// - compare by group contents (regardless of order), group behaves like a set (e.g. an ordered vector)

		// Get *set* of bait groups (i.e. bait + orthologs)
		// Grab neighbour groups for each of the baits via correlations (it's more memory efficient to do this by species)
		// union all groups while keeping track of their owners. (group, owners). Group is a node, each owner is a bait node, there is an edge between any group and any owner(=a single bait)

		// How to track owners:
		// map<Group, owners>; add to that map while running through all baits. Then each group will be shown in maximal groups of owners, which is fairly ideal really

		// TODO First intersect between all, then recurse by: for each left out bait do it again and exclude all those already put in a 'cluster'
		// TODO make intersections gathering efficient once all features are implemented

		//////////////////////
		// Output cytoscape files

		string network_name = "abiotic"; // TODO

		// sif file
		{
			ofstream out(network_name + ".sif");
			out.exceptions(ofstream::failbit | ofstream::badbit);

			for (auto bait : baits_) {
				out << gene_expressions->get_gene_name(bait) << "\n";
				for (auto neigh : bait_neighbours[bait]) {
					out << gene_expressions->get_gene_name(bait) << "\tpd\t" << gene_expressions->get_gene_name(neigh) << "\n";
				}
			}
		}

		string install_dir = "/home/limyreth/doc/internship/deep_blue_genome"; // TODO don't hardcode, instead install_dir/... or something. with structure: ./bin, ./data; so dirname(argv[0])/..
		install_dir + "/data/templates/coexpr_vizmap.props";
	});
}
