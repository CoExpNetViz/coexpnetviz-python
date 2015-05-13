/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/filesystem.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/util/printer.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

Database::Database(std::string path, bool start_fresh)
:	unknown_gene_collection(*this), database_path(std::move(path))
{
	auto main_file = get_main_file();
	if (!start_fresh && boost::filesystem::exists(main_file)) {
		Serialization::load_from_binary(main_file, *this);
	}
}

GeneVariant& Database::get_gene_variant(const std::string& name) {
	auto variant = try_get_gene_variant(name);
	if (variant)
		return *variant;
	else
		throw NotFoundException("Gene not part of a known gene collection: " + name); // TODO we now have a default gene collection for unknown genes, so this can no longer happen
}

GeneVariant* Database::try_get_gene_variant(const std::string& name) {
	for (auto& gene_collection : gene_collections) {
		auto variant = gene_collection->try_get_gene_variant(name);
		if (variant) {
			return variant;
		}
	}
	return &unknown_gene_collection.get_gene_variant(name); // Note: this should never fail (it's a bug otherwise)
}

OrthologGroup& Database::add_ortholog_group(const GeneFamilyId& external_id) {
	return add_ortholog_group(make_unique<OrthologGroup>(external_id));
}

OrthologGroup& Database::add_ortholog_group() {
	return add_ortholog_group(make_unique<OrthologGroup>());
}

OrthologGroup& Database::add_ortholog_group(std::unique_ptr<OrthologGroup>&& group_) {
	ortholog_groups.emplace_front(std::move(group_));
	auto& group = *ortholog_groups.front();
	group.set_iterator(ortholog_groups.begin());
	return group;
}

GeneCollection& Database::get_gene_collection(std::string name) {
	to_lower(name);
	return **find_if(gene_collections.begin(), gene_collections.end(), [name](unique_ptr<GeneCollection>& gene_collection) {
		std::string name_ = gene_collection->get_name();
		to_lower(name_);
		return name_ == name;
	});
}

void Database::erase(OrthologGroups::iterator it) {
	ortholog_groups.erase(it);
}

string Database::get_main_file() const {
	return database_path + "/db";
}

void Database::add(std::unique_ptr<GeneCollection>&& gene_collection) {
	auto it = find_if(gene_collections.begin(), gene_collections.end(), [&gene_collection](unique_ptr<GeneCollection>& g) {
		return g->get_name() == gene_collection->get_name();
	});
	ensure(it == gene_collections.end(),
			"Gene collection with name '" + gene_collection->get_name() + "' already exists",
			ErrorType::GENERIC
	);
	gene_collections.emplace_back(std::move(gene_collection));
}

GeneExpressionMatrix& Database::get_gene_expression_matrix(std::string name) {
	to_lower(name);
	return *gene_expression_matrices.at(name);
}

GeneExpressionMatrix& Database::add(unique_ptr<GeneExpressionMatrix>&& matrix) {
	ensure(gene_expression_matrices.find(matrix->get_name()) == gene_expression_matrices.end(),
			(make_string() << "Cannot add 2 gene expression matrices with the same name: matrix '" << matrix->get_name() << "'").str(),
			ErrorType::GENERIC
	);

	std::string name_lower = matrix->get_name();
	to_lower(name_lower);
	return *gene_expression_matrices.emplace(name_lower, std::move(matrix)).first->second;
}

void Database::save() {
	cout << "Saving database...\n";
	Serialization::save_to_binary(get_main_file(), *this);
}

unordered_set<const Gene*> Database::get_genes() const {
	unordered_set<const Gene*> all_genes;
	for (auto& collection : gene_collections) {
		boost::insert(all_genes, collection->get_genes() | referenced);
	}
	boost::insert(all_genes, unknown_gene_collection.get_genes() | referenced);
	return all_genes;
}

//TODO cerr flushes output at end of every (<< << <<) expression. So may not want to use it for frequent warnings. Should start using a proper logging library anyway...
void Database::verify() {
	using namespace boost::adaptors;
	using namespace std::placeholders;

	unordered_set<const Gene*> all_genes;
	boost::insert(all_genes, get_genes());

	auto soft_abort = []() {
		throw runtime_error("soft abort");
	};

	// No 2 genes have the same name
	cout << "No 2 gene pointers have the same name?" << endl;
	{
		vector<string> names;
		auto get_name = std::bind(&Gene::get_name, _1);
		auto lowered = [](string str) -> string {
			return to_lower(str);
		};
		boost::push_back(names, all_genes | transformed(get_name) | transformed(lowered));

		cout << names.size() << endl;

		auto&& duplicates = boost::unique<boost::return_found_end>(boost::sort(names));
		if (!boost::empty(duplicates)) {
			cerr << "Multiple gene pointers with same name: " << intercalate(", ", duplicates) << "\n";
			assert(false);
		}
	}

	// No gene present in multiple ortholog families
	cout << "No gene present in multiple ortholog families?" << endl;
	{
		auto get_groups_of = [this](const Gene& gene) {
			vector<const OrthologGroup*> groups;

			auto contains_gene = make_function([&gene](const OrthologGroup& group) {
				return contains(group.get_genes(), &gene);
			});

			boost::push_back(groups, ortholog_groups | indirected | filtered(contains_gene) | referenced);
			return groups;
		};

		unordered_set<Gene*> genes;
		size_t largest_group = 0;
		for (auto& group : ortholog_groups) {
			if (group->size() == 0) {
				cout << "Empty group!" << endl;
				soft_abort();
			}

			largest_group = max(largest_group, boost::size(group->get_genes()));
			for (auto gene : group->get_genes()) {
				if (!genes.emplace(gene).second) {
					// found duplicate gene
					auto groups = get_groups_of(*gene);
					cout << *gene << " present in multiple ortho groups: " << intercalate(", ", groups) << endl;
					soft_abort();
				}
			}
		}

		if (all_genes.size() > genes.size()) {
			cout << all_genes.size() - genes.size() << " genes aren't part of an ortholog group, not even a singleton group";
			soft_abort();
		}
		else if (all_genes.size() < genes.size()) {
			cout << genes.size() - all_genes.size() << " genes are present in an ortholog group, but not in a gene collection => they have no gene collection (not even the unknown gene collection)";
			soft_abort();
		}

		cout << "Max group size: " << largest_group << endl;
		cout << "Number of groups: " << boost::size(ortholog_groups) << endl;
	}

	// TODO no 2 ortholog groups should have the same external id

	cout << "Verification complete, all is well" << endl;
}

} // end namespace
