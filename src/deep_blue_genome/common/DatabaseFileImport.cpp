// Author: Tim Diels <timdiels.m@gmail.com>

#include "DatabaseFileImport.h"
#include <deep_blue_genome/common/TabGrammarRules.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/util.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

// TODO don't assume input it correct in plain inputs.
// TODO input validation on all read plain files, allow easier formats. Plain = clean, well-documented format. Bin = fast binary format.

void DatabaseFileImport::add_gene_mappings(const std::string& path, Database& database) {
	read_file(path, [&database](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto query = database.prepare("INSERT INTO gene_mapping (gene1_id, gene2_id) VALUES (%0q, %1q)");
		query.parse();

		auto on_line = [&database, &query](const std::vector<std::string>& line) {
			ensure(line.size() == 2,
					(make_string() << "Encountered line in mapping with " << line.size() << " columns").str(),
					ErrorType::GENERIC
			);

			auto gene1 = database.get_gene_by_name(line.at(0));
			auto gene2 = database.get_gene_by_name(line.at(1));
			ensure(gene1.get_gene_collection_id() != gene2.get_gene_collection_id(),
					(make_string() << "Encountered mapping between 2 genes of the same gene collection: " << gene1.get_name() << ", " << gene2.get_name()).str(),
					ErrorType::GENERIC
			);

			// TODO if exists, UPDATE with warning instead of INSERT
			query.execute(gene1.get_id(), gene2.get_id());
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

void DatabaseFileImport::add_functional_annotations(const string& path, Database& database) {
	read_file(path, [&database](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto query = database.prepare("UPDATE gene SET functional_annotation = %0q WHERE id = %1q");
		query.parse();

		auto on_line = [&database, &query](const std::vector<std::string>& line) {
			ensure(line.size() == 2,
					(make_string() << "Expected line with 2 columns, but got " << line.size() << " columns").str(),
					ErrorType::GENERIC
			);

			auto gene = database.get_gene_by_name(line.at(0));
			auto description = line.at(1);

			query.execute(description, gene.get_id());
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

void DatabaseFileImport::add_orthologs(const std::string& path, Database& database) {
	read_file(path, [&database](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		// Init ortholog group to first available group id
		auto query = database.prepare("SELECT max(ortholog_group) FROM gene");
		auto result = query.store();
		OrthologGroupId ortholog_group;
		if (result.num_rows() == 0) {
			ortholog_group = 1;
		}
		else {
			assert(result.num_rows() == 1);
			auto row = *result.begin();
			ortholog_group = row[0] + 1;
		}


		// Assign ortholog groups
		query = database.prepare("UPDATE gene SET ortholog_group = %0q WHERE id = %1q");
		query.parse();

		auto on_line = [&database, &query, &ortholog_group](const std::vector<std::string>& line) {
			ensure(line.size() < 2,
					(make_string() << "Encountered line in mapping with " << line.size() << " < 2 columns").str(),
					ErrorType::GENERIC
			);

			for (auto& name : line) {
				try {
					auto gene = database.get_gene_by_name(name); // Note: doing get gene by name to ensure it exists when we'll update it
					query.execute(ortholog_group, gene.get_id());
				}
				catch (const NotFoundException&) {
					;// TODO display warning like Database.cpp used to
				}
			}

			ortholog_group++;
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

}  // end namespace
