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

/**
 * Compares 2 sets of ortholog families, printing out stats
 */

#include <cmath>
#include <iostream>
#include <fstream>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/container/set.hpp>
#include <boost/function_output_iterator.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/util/printer.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using boost::container::flat_set;

flat_set<OrthologGroup*> read_orthologs(Database& database, std::string path) { // TODO refactor: this is a copy paste and change
	cout << "Loading orthologs '" << path << "'\n";
	flat_set<OrthologGroup*> groups;

	read_file(path, [&database, path, &groups](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [path, &database, &groups](const std::vector<std::string>& line) {
			if (line.size() < 3) {
				cout << "Warning: Encountered line in ortholog file with " << line.size() << " < 3 columns\n";
				return;
			}

			auto& group = database.add_ortholog_group(GeneFamilyId(path, line.at(0)));
			groups.emplace(&group);

			for (int i=1; i < line.size(); i++) {
				auto& name = line.at(i);
				try {
					try {
						auto& gene = database.get_gene_variant(name).as_gene();
						group.add(gene);
					}
					catch(const TypedException& e) {
						if (e.get_type() != ErrorType::SPLICE_VARIANT_INSTEAD_OF_GENE) {
							throw;
						}
						cout << "Warning: ignoring splice variant in orthologs file: " << name << "\n";
					}
				}
				catch (const NotFoundException&) {
				}
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});

	return groups;
}

double get_score(const OrthologGroup& group1, const OrthologGroup& group2) {
	long intersection_size = 0;
	auto&& counter = boost::make_function_output_iterator([&intersection_size](const Gene*){
		++intersection_size;
	});
	boost::set_intersection(group1.get_genes(), group2.get_genes(), counter);

	double jaccard = (double)intersection_size / (group1.size() + group2.size() - intersection_size);
	return max(max(jaccard, (double)intersection_size / group1.size()), (double)intersection_size / group2.size());
}

void print_stats(flat_set<OrthologGroup*> groups1, flat_set<OrthologGroup*> groups2) {
	std::array<long, 50> bin_counts; // number of scores that fall in the bin
	boost::fill(bin_counts, 0);
	double step = 1.0 / 50.0; // amount of space between bins, first bin is the range [0, step[, last bin is [1-step,1]

	// TODO highly parallelisable, could use TBB but then at least this thing should be made GPL since TBB is GPLv2
	int i=0;
	for (auto group1 : groups1) {
		for (auto group2 : groups2) {
			double score = get_score(*group1, *group2);
			int bin_index;
			if (score >= 1.0) {  // >1.0 to allow some numeric error
				// lump 1.0 together in last bin as well
				assert(score < 1.0 + 1.0e-6);  // don't allow too big a numeric error
				bin_index = bin_counts.size()-1;
			}
			else {
				bin_index = static_cast<int>(score / step); // also floors
			}

			bin_counts.at(bin_index)++;
		}
		cout << i++ << endl;
	}

	cout << intercalate(",", bin_counts) << endl;
}

int main(int argc, char** argv) {
	graceful_main([argc, argv]() {
		cout << "Warning: This program needs further modification to be reusable." << endl;
		Database database("tmpdb", true);
		auto&& groups1 = read_orthologs(database, argv[1]);
		auto&& groups2 = read_orthologs(database, argv[2]);
		print_stats(groups1, groups2);
	});
}
