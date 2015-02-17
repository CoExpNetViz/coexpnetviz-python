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
	// TODO splice variants are the same gene, but depending on the variant the mapping is different. There's also one 'variant' that includes non-coding regions as well. Should map: (gene, splice variant) -> (gene, splice variant).
	// And even then it can still happen to map to multiple. Maybe there's info on which splice variant a certain gene name is, e.g. Os*g* seems to never have a .num suffix
	read_file(path, [&database](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		//database.execute("BEGIN"); // TODO may need a class that upon dtor does rollback if not committed yet
		auto query = database.prepare("INSERT INTO gene_mapping (gene_variant1_id, gene_variant2_id) VALUES (%0q, %1q)");
		query.parse();

		auto on_line = [&database, &query](const std::vector<std::string>& line) {
			cout << ".";
			cout.flush();
			ensure(line.size() >= 2,
					(make_string() << "Encountered line in mapping with " << line.size() << " < 2 columns").str(),
					ErrorType::GENERIC
			);

			auto gene_variant1 = database.get_gene_variant(line.at(0));
			for (int i=1; i<line.size(); i++) {
				auto gene_variant2 = database.get_gene_variant(line.at(i));

				ensure(gene_variant1.get_gene().get_gene_collection_id() != gene_variant2.get_gene().get_gene_collection_id(),
						(make_string() << "Encountered mapping between 2 genes of the same gene collection: " << gene_variant1.get_gene().get_name() << ", " << gene_variant2.get_gene().get_name()).str(),
						ErrorType::GENERIC
				);

				// TODO if exists, UPDATE with warning instead of INSERT
				query.execute(gene_variant1.get_id(), gene_variant2.get_id());
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		//database.execute("COMMIT");
		return begin;
	});

}

void DatabaseFileImport::add_functional_annotations(const string& path, Database& database) {
	read_file(path, [&database](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto query = database.prepare("UPDATE gene_variant SET functional_annotation = %0q WHERE id = %1q");
		query.parse();

		auto on_line = [&database, &query](const std::vector<std::string>& line) {
			ensure(line.size() == 2,
					(make_string() << "Expected line with 2 columns, but got " << line.size() << " columns").str(),
					ErrorType::GENERIC
			);

			auto gene_variant = database.get_gene_variant(line.at(0));
			auto description = line.at(1);

			query.execute(description, gene_variant.get_id());
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

void DatabaseFileImport::add_orthologs(const std::string& path, Database& database) {
	// TODO when encountering 2 groups (=lines) of orthologs with an overlap, throw an error
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
			ensure(line.size() >= 2,
					(make_string() << "Encountered line in mapping with " << line.size() << " < 2 columns").str(),
					ErrorType::GENERIC
			);

			for (auto& name : line) {
				try {
					auto gene_variant = database.get_gene_variant(name); // Note: doing get gene variant to ensure it exists when we'll update it
					ensure(!gene_variant.is_splice_variant() || gene_variant.get_splice_variant_id() == 1,  // disallow specification of splice variants, except for splice variant 1 which gets interpreted as the entire gene. (The latter is an act of lenience)
							(make_string() << "Member of ortholog group must be a gene, instead got: " << name).str(),
							ErrorType::GENERIC
					);
					auto gene = gene_variant.get_gene();
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
