// Author: Tim Diels <timdiels.m@gmail.com>

// TODO gene collection, expression matrix, ... names should be stored with case but matched without it

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

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
	Group(Database& database, IterableT genes)
	{
		this->genes.reserve(genes.size());
		for (auto& gene_id : genes) {
			this->genes.emplace_back(database.get_gene(gene_id));
			name += this->genes.back().get_name() + ";";
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
	vector<Gene> genes;
	set<Owner> owners;  // ordered set of bait genes to which these genes are correlated
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
	template <class GeneCollectionsIterable>
	GeneGroups(Database& database, const GeneCollectionsIterable& all_gene_collections)
	:	database(database), all_gene_collections(all_gene_collections.begin(), all_gene_collections.end())
	{
	}

	/**
	 * Get Group of gene
	 */
	Group& get(const Gene& gene) {
		auto group_id = gene.get_ortholog_group_id();
		auto it = groups.find(group_id);
		if (it == groups.end()) {
			// make group
			auto p = groups.emplace(piecewise_construct,
					forward_as_tuple(group_id),
					forward_as_tuple(database, database.get_orthologs(gene.get_id(), all_gene_collections))
			);
			return p.first->second;
		}
		else {
			return it->second;
		}
	}

private:
	Database& database;
	vector<GeneCollectionId> all_gene_collections;
	unordered_map<OrthologGroupId, Group> groups;
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
		Database database;

		// Read yaml
		YAML::Node job_node = YAML::LoadFile(argv[1]);
		string baits_path = job_node["baits"].as<string>();

		double negative_treshold = job_node["negative_treshold"].as<double>();
		ensure(fabs(negative_treshold) <= 1.0+1e-7, "negative_treshold must be a double between -1 and 1", ErrorType::GENERIC);

		double positive_treshold = job_node["positive_treshold"].as<double>();
		ensure(fabs(positive_treshold) <= 1.0+1e-7, "positive_treshold must be a double between -1 and 1", ErrorType::GENERIC);

		unordered_set<GeneCollectionId> all_gene_collections; // all collections needed for this run
		map<GeneCollectionId, shared_ptr<GeneExpressionMatrix>> expression_matrices; // gene collection -> matrix
		for (auto matrix_node : job_node["expression_matrices"]) {
			GeneCollectionId gene_collection = database.get_gene_collection_id(matrix_node["gene_collection"].as<string>());
			string matrix_name = matrix_node["name"].as<string>();

			bool created = all_gene_collections.emplace(gene_collection).second;
			ensure(created, "Specified multiple matrices of the same gene collection", ErrorType::GENERIC);

			auto matrix_id = database.get_gene_expression_matrix_id(gene_collection, matrix_name);
			expression_matrices.emplace(gene_collection, database.get_gene_expression_matrix(matrix_id));
		}

		// Gene groups
		GeneGroups groups(database, all_gene_collections);

		// Load baits
		vector<Group*> baits; // list of distinct baits
		{
			Baits baits_(baits_path);
			for (auto& gene_name : baits_.get_genes()) {
				Gene gene = database.get_gene(gene_name);
				auto& group = groups.get(gene);

				// if a member is missing from its corresponding expression matrix, drop the bait
				bool drop = false;
				for (auto& gene : group) {
					if (!expression_matrices.at(gene.get_gene_collection_id())->has_gene(gene.get_id())) {
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

		// If baits are dropped, gene collections might drop out too, so we rebuild all_gene_collections here
		all_gene_collections.clear();
		for (auto group : baits) {
			for (auto& bait : *group) {
				all_gene_collections.emplace(bait.get_gene_collection_id());
			}
		}

		// Grab union of neighbours of each bait, where neighbour relation is sufficient (anti-)correlation
		std::vector<Group*> neighbours;
		unordered_map<GeneCollectionId, vector<size_type>> bait_indices; // species -> gene indices of baits
		for (auto group : baits) {
			for (auto bait : *group) {
				// Calculate correlations
				auto matrix = expression_matrices.at(bait.get_gene_collection_id());
				bait_indices[bait.get_gene_collection_id()].emplace_back(matrix->get_gene_row(bait.get_id()));
			}
		}
		for (auto& gene_collection_id : all_gene_collections) {
			auto expression_matrix = expression_matrices.at(gene_collection_id);
			auto& indices = bait_indices.at(gene_collection_id);

			GeneCorrelationMatrix correlations(*expression_matrix, indices);
			auto& correlations_ = correlations.get();

			// Make edges to nodes with sufficient correlation
			for (auto row_index : indices) {
				auto col_index = correlations.get_column_index(row_index);
				auto bait_id = expression_matrix->get_gene_id(row_index);
				auto bait = database.get_gene(bait_id);
				for (size_type gene = 0; gene < correlations_.size1() && gene != row_index; gene++) {
					auto corr = correlations_(gene, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						auto row_gene = database.get_gene(expression_matrix->get_gene_id(gene));
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
