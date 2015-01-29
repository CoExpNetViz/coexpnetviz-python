// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
#include <deep_blue_genome/common/GeneMapping.h>
#include <deep_blue_genome/common/GeneMappingId.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

// TODO if many applications require to know orthologs for one gene to that of all other species, we might want to redesign the ortholog mapping files to source_species -> all_other_species

class GeneExpressionMatrixId // TODO unused atm
{
public:
	GeneExpressionMatrixId(std::string species, std::string name)
	:	species(species), name(name)
	{
	}

	const string& get_species() const;
	const string& get_name() const;

private:
	std::string species;
	std::string name;
};

const string& GeneExpressionMatrixId::get_species() const {
	return species;
}

const string& GeneExpressionMatrixId::get_name() const {
	return name;
}

class Gene
{
public:
	Gene(const string& species, const string& name);

	const string& get_species() const;
	const string& get_name() const;

	bool operator==(const Gene& other) const {
		return name == other.name;
	}

	bool operator<(const Gene& other) const {
		return name < other.name;
	}

private:
	string species;
	string name;
};

Gene::Gene(const string& species, const string& name)
:	species(species), name(name)
{
}

const string& Gene::get_species() const {
	return species;
}

const string& Gene::get_name() const {
	return name;
}

namespace std {
template <> struct hash<Gene>
{
	size_t operator()(const Gene& x) const
	{
		size_t hash = 0;
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_name());
		return hash;
	}
};
} // end namespace

/**
 * Group of genes which are each other's orthologs
 */
class Group : public boost::noncopyable
{
public:
	/**
	 * @param genes Iterable of distinct genes
	 */
	template<class IterableT>
	Group(IterableT genes)
	:	genes(genes.begin(), genes.end())
	{
	}

	bool operator==(const Group& other) const = delete;

	void add_owner(const Gene& owner) {
		owners.emplace_back(owner);
	}

	vector<Gene>::const_iterator begin() const;
	vector<Gene>::const_iterator end() const;

private:
	vector<Gene> genes; // ordered
	vector<Gene> owners; // bait genes to which these genes are correlated
};

vector<Gene>::const_iterator Group::begin() const {
	return genes.begin();
}

vector<Gene>::const_iterator Group::end() const {
	return genes.end();
}

class GeneGroups
{
public:
	template <class SpeciesIterable>
	GeneGroups(Database& database, const SpeciesIterable& all_species)
	:	all_species(all_species.begin(), all_species.end())
	{
		for (auto& source_species : all_species) {
			for (auto& target_species : all_species) {
				auto mapping = database.get_ortholog_mapping(GeneMappingId(source_species, target_species));
				ortholog_mappings.emplace(piecewise_construct, forward_as_tuple(source_species, target_species), forward_as_tuple(mapping));
			}
		}
	}

	/**
	 * Get Group of gene
	 */
	Group& get(const Gene& gene) {
		auto it = group_of_gene.find(gene);
		if (it == group_of_gene.end()) {
			// make group
			vector<Gene> orthologs;
			orthologs.emplace_back(gene);
			for (auto& target_species : all_species) {
				auto it = ortholog_mappings.find(GeneMappingId(gene.get_species(), target_species));
				assert(it != ortholog_mappings.end());
				assert(it->second);
				auto mapping = it->second;
				if (mapping->has(gene.get_name())) {
					auto& genes = mapping->get(gene.get_name());
					for (auto& gene : genes) {
						orthologs.emplace_back(target_species, gene);
					}
				}
			}
			sort(orthologs.begin(), orthologs.end());
			auto uniq_end = unique(orthologs.begin(), orthologs.end());
			groups.emplace_back(make_unique<Group>(make_iterable(orthologs.begin(), uniq_end)));
			auto& group = *groups.back();

			// add mappings
			for (auto& gene : group) {
				bool created = group_of_gene.emplace(gene, &group).second;
				assert(created);
			}

			// return
			return group;
		}
		else {
			return *it->second;
		}
	}

private:
	vector<string> all_species;
	unordered_map<GeneMappingId, shared_ptr<GeneMapping>> ortholog_mappings;
	unordered_map<Gene, Group*> group_of_gene;
	vector<unique_ptr<Group>> groups;
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
		map<string, shared_ptr<GeneExpressionMatrix>> expression_matrices; // species -> matrix
		for (auto matrix_node : job_node["expression_matrices"]) {
			database
			string species = matrix_node["species"].as<string>();
			string matrix_name = matrix_node["name"].as<string>();

			bool created = species_list.emplace(species).second;
			ensure(created, "Specified more than 1 matrix for the same species", ErrorType::GENERIC);

			expression_matrices.emplace(species, database.get_expression_matrix(species, matrix_name));
		}

		// Gene groups
		GeneGroups groups(database, species_list);

		// Load baits
		vector<Group*> baits; // list of distinct baits
		{
			Baits baits_(baits_path);
			for (auto& gene_name : baits_.get_genes()) {
				Gene gene(database.get_species_of_gene(gene_name), gene_name);
				auto& group = groups.get(gene);
				baits.emplace_back(&group);
			}
		}

		// Build species_baits map // TODO turn into a func that's called in next section
		map<string, vector<Gene>> species_baits; // species -> baits of species
		for (auto& expression_matrix_id : expression_matrices) {
			for (auto& group : baits) {
				for (auto& bait : *group) {
					species_baits[bait.get_species()].emplace_back(bait);
				}
			}
		}

		// Remove bait groups of which at least one member is missing from its corresponding expression matrix
		cout << "Warning: bait gene '" << << "' missing in expression matrix. Dropping bait and its orthologs." << "\n";

		// Grab union of neighbours of each bait, where neighbour relation is sufficient (anti-)correlation
		std::vector<Group> neighbours;
		for (auto& matrix_id : expression_matrices) { // by expression matrix since that saves memory
			auto& baits = species_baits[matrix_id.get_species()];

			// Calculate correlations
			auto gene_expressions = database.get_gene_expression_matrix(matrix_id.get_species(), matrix_id.get_name());

			std::vector<size_type> baits_;
			for (auto gene : baits) {
				if (gene_expressions->has_gene(gene.get_name())) {
					baits_.emplace_back(gene_expressions->get_gene_index(gene.get_name()));
				}
				else {
					// TODO remove bait group and all things
					;
				}
			}

			GeneCorrelationMatrix correlations(*gene_expressions, baits_);
			auto& correlations_ = correlations.get();

			// Make edges to nodes with sufficient correlation
			for (auto& bait : baits) {
				auto row_index = gene_expressions->get_gene_index(bait.get_name());
				auto col_index = correlations.get_column_index(row_index);
				for (size_type gene = 0; gene < correlations_.size1() && gene != row_index; gene++) {
					auto corr = correlations_(gene, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						Gene row_gene(matrix_id.get_species(), gene_expressions->get_gene_name(gene));
						groups.get(row_gene).add_owner(bait);
					}
				}
			}
		}

		//////////////////////
		// Output cytoscape files using: baits, neighbours

		string network_name = "abiotic"; // TODO

		// sif file
		/*{
			ofstream out(network_name + ".sif");
			out.exceptions(ofstream::failbit | ofstream::badbit);

			for (auto bait : baits_) {
				out << gene_expressions->get_gene_name(bait) << "\n";
				for (auto neigh : bait_neighbours[bait]) {
					out << gene_expressions->get_gene_name(bait) << "\tpd\t" << gene_expressions->get_gene_name(neigh) << "\n";
				}
			}
		}*/

		string install_dir = "/home/limyreth/doc/internship/deep_blue_genome"; // TODO don't hardcode, instead install_dir/... or something. with structure: ./bin, ./data; so dirname(argv[0])/..
		install_dir + "/data/templates/coexpr_vizmap.props";
	});
}
