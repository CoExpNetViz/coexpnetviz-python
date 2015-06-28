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

#include "DataFileImport.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string.hpp>
#include <deep_blue_genome/common/TabGrammarRules.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/ublas.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

// TODO don't assume input it correct in plain inputs.
// TODO input validation on all read plain files, allow easier formats. Plain = clean, well-documented format. Bin = fast binary format.

DataFileImport::DataFileImport(Database& db)
:	database(db)
{
}

void DataFileImport::add_gene_mappings(const std::string& path) {
	cout << "Loading gene mapping '" << path << "'\n";
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [this](const std::vector<std::string>& line) {
			ensure(line.size() >= 2,
					(make_string() << "Encountered line in mapping with " << line.size() << " < 2 columns").str(),
					ErrorType::GENERIC
			);

			auto& src = database.get_gene_variant(line.at(0)).get_dna_sequence();
			for (int i=1; i<line.size(); i++) {
				auto& dst = database.get_gene_variant(line.at(i)).get_dna_sequence();
				src.add_highly_similar(dst);
			}
		};

		TabGrammarRules rules(true);
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});

}

void DataFileImport::add_functional_annotations(const string& path) {
	cout << "Loading functional annotations '" << path << "'\n";
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [this](const std::vector<std::string>& line) {
			if (line.size() < 2) {
				// Incomplete line, ignore
				return;
			}

			ensure(line.size() == 2,
					(make_string() << "Expected line with 2 columns, but got " << line.size() << " columns").str(),
					ErrorType::GENERIC
			);

			auto& gene_variant = database.get_gene_variant(line.at(0));
			auto description = line.at(1);
			boost::algorithm::trim(description);
			if (!description.empty()) {
				gene_variant.set_functional_annotation(description);
			}
		};

		TabGrammarRules rules(true);
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

void DataFileImport::add_orthologs(std::string source_name, std::string path) {
	cout << "Loading orthologs '" << path << "'\n";

	read_file(path, [this, source_name](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		// Assign ortholog groups
		auto on_line = [this, source_name](const std::vector<std::string>& line) {
			if (line.size() < 3) {
				// Ignore singletons
				return;
			}

			auto& group = database.add_ortholog_group(GeneFamilyId(source_name, line.at(0)));

			for (int i=1; i < line.size(); i++) {
				auto& name = line.at(i);
				try {
					auto& gene = database.get_gene_variant(name).as_gene();
					gene.add_ortholog_group(group);
				}
				catch(const TypedException& e) { // TODO hierarchy on exceptions instead of the TypedException thing; though do add an get_error_code to them?
					if (e.get_type() != ErrorType::SPLICE_VARIANT_INSTEAD_OF_GENE) {
						throw;
					}
					cout << "Warning: ignoring splice variant in orthologs file: " << name << "\n";
				}
			}
		};

		TabGrammarRules rules(true);
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

GeneExpressionMatrix& DataFileImport::add_gene_expression_matrix(const std::string& name, const std::string& path) {
	cout << "Loading gene expression matrix '" << path << "'\n";
	auto gem = make_unique<GeneExpressionMatrix>();

	gem->name = name;
	read_file(path, [this, &gem, path](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto current = begin;
		TabGrammarRules rules(true);

		// Note: Since we don't know line count up front, we buffer lines in a vector before putting it in a matrix.
		// 		 Since lines can be skipped, we cannot trivially count the number of newlines.

		// parse header
		int column_count;
		{
			std::vector<std::string> header_items;
			parse(current, end, rules.line > eol, header_items);
			column_count = header_items.size() - 1;
		}

		// parse gene lines
		std::vector<std::vector<matrix::value_type>> lines; // values on each line, does not include skipped lines
		int line_number = 1;  // start at 1 because of header
		bool skip_line = false;

		auto on_new_gene = [this, &gem, &lines, &line_number, &skip_line, column_count](std::string name) { // start new line
			// TODO current ensure always constructs its error message, want to delay that (a lambda probably is needed)
			if(!(lines.empty() || lines.back().size()==column_count)) {
				ensure(false,
						(make_string() << "Line " << line_number << " (1-based, header included): expected "
										<< column_count << " columns, got " << lines.back().size()).str()
				);
			}

			line_number++;

			if (name.empty()) {
				skip_line = true;
				cout << "Warning: Line " << line_number << ": skipping line with empty gene name\n";
				return;
			}
			else {
				skip_line = false;
			}

			Gene& gene = database.get_gene_variant(name).get_gene();

			lines.emplace_back(vector<matrix::value_type>());
			lines.back().reserve(column_count);

			bool created = gem->gene_to_row.emplace(&gene, lines.size()-1).second;
			ensure(created,
					(make_string() << "Duplicate gene: " << name).str()
			);

			gem->row_to_gene.emplace_back(&gene);
		};

		auto on_gene_value = [&lines, &skip_line](double value) { // gene expression value
			if (!skip_line)
				lines.back().emplace_back(value);
		};

		parse(current, end, (rules.field[on_new_gene] > rules.separator > (double_[on_gene_value] % rules.separator)) % rules.line_separator);

		// Validate last line
		ensure(lines.back().size() == column_count,
				(make_string() << "Last (non-skipped) line: " << lines.back().size() << " values instead of " << column_count).str()
		);

		// Convert to matrix
		gem->expression_matrix.resize(lines.size(), column_count, false);
		for (matrix::size_type i=0; i<lines.size(); ++i) {
			auto&& line = lines.at(i);
			for (matrix::size_type j=0; j<column_count; j++) {
				gem->expression_matrix(i,j) = line.at(j);
			}
		}

		return current;
	});

	return database.add(move(gem));
}

// TODO did we drop variants along the way as we read in clusterings? If so, should we?
// TODO should clusters allow splice variants or not? This code hasn't decided yet; though cluster has, it uses genes
void DataFileImport::add_clustering(const std::string& name, const std::string& path, const std::string& expression_matrix_name) {
	cout << "Loading clustering '" << path << "'\n";

	auto clustering = make_unique<Clustering>();

	clustering->gene_collection = nullptr;
	clustering->expression_matrix = nullptr;
	clustering->name = name;

	// Load
	read_file(path, [this, &clustering](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		std::unordered_map<std::string, Cluster> clusters;

		auto on_cluster_item = [this, &clustering, &clusters](const std::vector<std::string>& line) {
			auto name = line.at(0);

			auto gene_variant = database.try_get_gene_variant(name);
			if (!gene_variant) {
				cerr << "Warning: Gene of unknown collection '" << name << "'\n";
				return;
			}

			if (!clustering->gene_collection) {
				clustering->gene_collection = &gene_variant->get_gene_collection();
			}
			else {
				ensure(clustering->gene_collection == &gene_variant->get_gene_collection(),
						"All genes in a clustering must be of the same gene collection. Conflicting gene: " + name,
						ErrorType::GENERIC
				);
			}

			Gene& gene = gene_variant->get_gene();

			auto cluster_name = line.at(1);
			auto it = clusters.find(cluster_name);
			if (it == clusters.end()) {
				it = clusters.emplace(piecewise_construct, make_tuple(cluster_name), make_tuple(cluster_name)).first;
			}
			auto& cluster = it->second;
			cluster.add(gene);
		};

		TabGrammarRules rules(true);
		parse(begin, end, rules.line[on_cluster_item] % eol);

		// Move clusters' values to this->clusters
		clustering->clusters.reserve(clusters.size());
		for(auto& p : clusters) {
			 clustering->clusters.emplace_back(std::move(p.second));
		}

		return begin;
	});

	// Check contents for uniqueness
	{
		auto genes_lt = [](Gene* a, Gene* b) {
			return a->get_name() < b->get_name();
		};

		auto genes_eq = [](Gene* a, Gene* b) {
			return a->get_name() == b->get_name();
		};

		vector<Gene*> all_genes;
		for (auto& cluster : clustering->clusters) {
			all_genes.insert(all_genes.end(), cluster.begin(), cluster.end());
		}
		sort(all_genes.begin(), all_genes.end(), genes_lt);
		auto unique_end = unique(all_genes.begin(), all_genes.end(), genes_eq);

		if (all_genes.end() != unique_end) {
			vector<string> gene_names;
			transform(unique_end, all_genes.end(), back_inserter(gene_names), [](Gene* gene) {
				return gene->get_name();
			});

			ostringstream str;
			str << "Clustering contains some genes more than once: ";
			copy(gene_names.begin(), gene_names.end(), ostream_iterator<std::string>(str, ", "));
			ensure(false, str.str(), ErrorType::GENERIC);
		}
	}

	if (!expression_matrix_name.empty()) {
		clustering->expression_matrix = &database.get_gene_expression_matrix(expression_matrix_name);
	}

	clustering->get_gene_collection().add_clustering(move(clustering));
}

}  // end namespace

