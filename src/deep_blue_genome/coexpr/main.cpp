// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
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
 * A bait gene that 'owns' another gene by correlating sufficiently to it
 *
 * Data class of Group.
 */
class Owner {
public:
	Owner(const Gene& owner, double correlation)
	:	owner(owner), correlation(correlation)
	{
	}

	const Gene& get_gene() const;
	double get_correlation() const;

	bool operator<(const Owner& other) const {
		return owner < other.owner;
	}

private:
	Gene owner;
	double correlation; // correlation to owner
};

const Gene& Owner::get_gene() const {
	return owner;
}

double Owner::get_correlation() const {
	return correlation;
}

namespace std {
template <> struct hash<Owner>
{
	size_t operator()(const Owner& x) const
	{
		size_t hash = 0;
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_gene());
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
		for (auto& gene : genes) {
			name += gene.get_name() + ";";
		}
	}

	bool operator==(const Group& other) const = delete;

	string get_name() const;

	void add_owner(const Gene& owner, double correlation) {
		owners.emplace(owner, correlation);
	}

	const set<Owner>& get_owners() {
		return owners;
	}

	vector<Gene>::const_iterator begin() const;
	vector<Gene>::const_iterator end() const;

private:
	vector<Gene> genes; // ordered
	set<Owner> owners; // ordered set of bait genes to which these genes are correlated
	string name;
};

vector<Gene>::const_iterator Group::begin() const {
	return genes.begin();
}

vector<Gene>::const_iterator Group::end() const {
	return genes.end();
}

string Group::get_name() const {
	return name;
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
				auto mapping = ortholog_mappings.at(GeneMappingId(gene.get_species(), target_species));
				assert(mapping);
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

		unordered_set<std::string> all_species;
		map<string, shared_ptr<GeneExpressionMatrix>> expression_matrices; // species -> matrix
		for (auto matrix_node : job_node["expression_matrices"]) {
			string species = matrix_node["species"].as<string>();
			string matrix_name = matrix_node["name"].as<string>();

			bool created = all_species.emplace(species).second;
			ensure(created, "Specified more than 1 matrix for the same species", ErrorType::GENERIC);

			expression_matrices.emplace(species, database.get_gene_expression_matrix(species, matrix_name));
		}

		// Gene groups
		GeneGroups groups(database, all_species);

		// Load baits
		vector<Group*> baits; // list of distinct baits
		{
			Baits baits_(baits_path);
			for (auto& gene_name : baits_.get_genes()) {
				Gene gene(database.get_species_of_gene(gene_name), gene_name);
				auto& group = groups.get(gene);

				// if a member is missing from its corresponding expression matrix, drop the bait
				bool drop = false;
				for (auto& gene : group) {
					if (!expression_matrices.at(gene.get_species())->has_gene(gene.get_name())) {
						cout << "Warning: bait gene '" << gene.get_name() << "' missing in expression matrix. Dropping bait and its orthologs." << "\n";
						drop = true;
						break;
					}
				}

				if (!drop) {
					baits.emplace_back(&group);
				}
			}
		}

		// If baits are dropped, species might drop out too, so we rebuild all_species here
		all_species.clear();
		for (auto group : baits) {
			for (auto& bait : *group) {
				auto& species = bait.get_species();
				all_species.emplace(species);
			}
		}

		// Grab union of neighbours of each bait, where neighbour relation is sufficient (anti-)correlation
		std::vector<Group*> neighbours;
		unordered_map<string, vector<size_type>> bait_indices; // species -> gene indices of baits
		for (auto group : baits) {
			for (auto bait : *group) {
				// Calculate correlations
				auto matrix = expression_matrices.at(bait.get_species());
				bait_indices[bait.get_species()].emplace_back(matrix->get_gene_index(bait.get_name()));
			}
		}
		for (auto& species : all_species) {
			auto expression_matrix = expression_matrices.at(species);
			auto& indices = bait_indices.at(species);

			GeneCorrelationMatrix correlations(*expression_matrix, indices);
			auto& correlations_ = correlations.get();

			// Make edges to nodes with sufficient correlation
			for (auto row_index : indices) {
				auto col_index = correlations.get_column_index(row_index);
				auto bait_name = expression_matrix->get_gene_name(row_index);
				auto bait = Gene(species, bait_name);
				for (size_type gene = 0; gene < correlations_.size1() && gene != row_index; gene++) {
					auto corr = correlations_(gene, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						Gene row_gene(species, expression_matrix->get_gene_name(gene));
						auto& group = groups.get(row_gene);
						group.add_owner(bait, corr);
						neighbours.emplace_back(&group);
					}
				}
			}
		}

		// TODO assert distinct(baits), distinct(neighbours)

		//////////////////////
		// Output cytoscape files using: baits, neighbours

		string install_dir = "/home/limyreth/doc/internship/deep_blue_genome"; // TODO don't hardcode, instead install_dir/... or something. with structure: ./bin, ./data; so dirname(argv[0])/..
		string network_name = "network1"; // TODO

		ofstream out_sif(network_name + ".sif");
		out_sif.exceptions(ofstream::failbit | ofstream::badbit);

		ofstream out_edge_attr(network_name + ".eda");
		out_edge_attr.exceptions(ofstream::failbit | ofstream::badbit);
		out_edge_attr << "R_VALUE (class=Double)" << "\n";

		ofstream out_node_attr(network_name + ".noa");
		out_node_attr.exceptions(ofstream::failbit | ofstream::badbit);
		out_node_attr << "Node_Information (class=java.lang.String)" << "\n";

		auto vizmap = network_name + ".props";
		boost::filesystem::copy_file(install_dir + "/data/templates/coexpr_vizmap.props", vizmap, boost::filesystem::copy_option::overwrite_if_exists);
		ofstream out_vizmap(vizmap, ios::app);
		out_vizmap.exceptions(ofstream::failbit | ofstream::badbit);

		for (auto group : baits) {
			for (auto bait : *group) {
				out_sif << bait.get_name() << "\n";
				out_node_attr << bait.get_name() << " = Bait\n";
			}
		}

		unordered_set<string> owner_groups;
		for (auto neigh : neighbours) {
			string owner_group_name;
			for (auto& owner : neigh->get_owners()) {
				auto owner_name = owner.get_gene().get_name();
				owner_group_name += owner_name + ";";
				out_sif << owner_name << "\tpd\t" << neigh->get_name() << "\n";
				out_edge_attr << owner_name << " pd " << neigh->get_name() << " = " << owner.get_correlation() << "\n";
			}
			owner_groups.emplace(owner_group_name);
			out_node_attr << neigh->get_name() << " = " << owner_group_name << "\n";
		}

		// Pick size(owner_groups) colors
		// TODO include in thesis: alg for having N maximally orthogonal colours (though it's not a real fancy alg or idea...). We did the math on paper, translate it to latex
		//colors;

		for (auto& group_name : owner_groups) {
			// TODO color more cleverly, perhaps this helps: http://www.nps.edu/faculty/olsen/Remote_sensing/SPIE_2000_tyo_display.pdf
			out_vizmap << "nodeFillColorCalculator.default-Node\\ Color-Discrete\\ Mapper.mapping.map." << group_name << "=" << rand() % 256 << "," << rand() % 256 << "," << rand() % 256 << "\n";

			// size
			out_vizmap << "nodeUniformSizeCalculator.default-Node\\ Size-Discrete\\ Mapper.mapping.map." << group_name << "=80.0\n";
		}
	});
}
