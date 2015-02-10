// Author: Tim Diels <timdiels.m@gmail.com>

#include <ncurses.h> // best documentation is: http://www.tldp.org/LDP/lpg/node85.html
#include <iostream>
#include <unordered_map>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/TabGrammarRules.h>


using namespace std;
using namespace DEEP_BLUE_GENOME; // TODO put common classes in DEEP_BLUE_GENOME::COMMON ?

namespace DEEP_BLUE_GENOME {
namespace DATABASE {

class NCurses {
public:
	NCurses() {
		initscr();;
	}

	~NCurses() {
		endwin();
		//refresh();
	}
};

void load_plaza_orthologs(Database& database, std::string path) {
	unordered_map<GeneMappingId, shared_ptr<GeneMapping>> mappings;
	unordered_map<string, Canonicaliser> canonicalisers;

	auto canonicalise = [&database, &canonicalisers](const std::string& species, const std::string& gene) {
		if (canonicalisers.find(species) == canonicalisers.end()) {
			canonicalisers.emplace(piecewise_construct, forward_as_tuple(species), forward_as_tuple(database, species));
		}
		return canonicalisers.find(species)->second.get(gene);
	};

	// Build mappings
	read_file(path, [&mappings, &canonicalise, &database](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		NCurses curses;

		unsigned int lines_read = 1;
		auto on_line = [&mappings, &canonicalise, &database, &lines_read](const std::vector<std::string>& line) {
			// show progress
			int x, y;
			getyx(stdscr, y, x);
			move(y, 0);
			clrtoeol();
			printw("%d / ? lines", lines_read++);
			refresh();

			try {
				auto source_gene = line.at(0);
				std::string source_species = database.get_species_of_gene(source_gene);
				auto source_genes = canonicalise(source_species, source_gene);

				vector<std::string> mapped_names(line.begin()+1, line.end());
				for (auto& target_gene : mapped_names) {
					try {
						std::string target_species = database.get_species_of_gene(target_gene);
						auto target_genes = canonicalise(target_species, target_gene);

						auto& mapping = mappings[GeneMappingId(source_species, target_species)];
						//cout << source_gene << ":" << source_species << " " << target_gene << ":" << target_species << "\n";
						if (!mapping.get()) {
							mapping = make_shared<GeneMapping>();
						}

						for (auto& source_gene : source_genes) {
							for (auto& target_gene : target_genes) {
								mapping->add(source_gene, target_gene);
							}
						}
					}
					catch (const SpeciesNotFoundException&) {
						// skip
					}
				}
			}
			catch (const SpeciesNotFoundException&) {
				// skip
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});

	// Save mappings
	for (auto& p : mappings) {
		database.update_ortholog_mapping(p.first, p.second);
	}
}

}} // end namespace
