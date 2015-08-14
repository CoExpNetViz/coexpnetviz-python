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
 * Compares 2 sets of ortholog families, printing out stats, ... (changes all the time, lots of hacks and copy pasta)
 */

#include <cmath>
#include <iostream>
#include <fstream>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/container/set.hpp>
#include <boost/function_output_iterator.hpp>
#include <deep_blue_genome/common/reader/MCL.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/common/TabGrammarRules.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/util/printer.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using boost::container::flat_set;

double get_score(const OrthologGroup& group1, const OrthologGroup& group2) {
	long intersection_size = 0;
	auto&& counter = boost::make_function_output_iterator([&intersection_size](const Gene*){
		++intersection_size;
	});
	boost::set_intersection(group1.get_genes(), group2.get_genes(), counter);

	double jaccard = (double)intersection_size / (group1.size() + group2.size() - intersection_size);
	return max(max(jaccard, (double)intersection_size / group1.size()), (double)intersection_size / group2.size());
}

// unused now
void print_stats(flat_set<OrthologGroup*> groups1, flat_set<OrthologGroup*> groups2) {
	std::array<long, 50> bin_counts; // number of scores that fall in the bin
	boost::fill(bin_counts, 0);
	double step = 1.0 / 50.0; // amount of space between bins, first bin is the range [0, step[, last bin is [1-step,1]

	// TODO highly parallelisable, could use TBB but then at least this thing should be made GPL since TBB is GPLv2
	int i=0;
	cout << groups1.size() << " x " << groups2.size() << endl;
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

/**
 * Write graph to file in MCL abc format
 */
void write_graph(flat_set<OrthologGroup*> groups1, flat_set<OrthologGroup*> groups2) {
	// TODO this only works for sets of groups with no overlap in group ids
	ofstream out("graph");
	out.exceptions(ofstream::failbit | ofstream::badbit);
	auto get_name = [](const OrthologGroup& group) {
		// assuming just one external id, ignoring source
		return boost::begin(group.get_external_ids())->get_id();
	};
	size_t i=0;
	size_t milestone = groups1.size() / 100;
	int progress=0;
	for (auto group1 : groups1) {
		for (auto group2 : groups2) {
			double score = get_score(*group1, *group2);
			if (score > 0.0) {
				out << get_name(*group1) << "\t" << get_name(*group2) << "\t" << score << "\n";
			}
		}
		if (i == milestone) {
			i = 0;
			cout << ++progress << "%" << endl;
		}
		else {
			++i;
		}
	}
}

// TODO this adds merged orthologs to the database, we want to get a less hacked together way of doing this!
int main(int argc, char** argv) {
	graceful_main([argc, argv]() {
		cout << "Warning: This program needs further modification to be reusable." << endl;
		Database database(argv[1], false);
		//print_stats(groups1, groups2);
		//write_graph(groups1, groups2);

		// Load families
		DataFileImport reader(database);
		reader.add_orthologs("monocots", argv[2]);
		reader.add_orthologs("dicots", argv[3]);
		cout << "Families (before): " << boost::distance(database.get_ortholog_groups()) << endl;

		// Load family clustering
		DEEP_BLUE_GENOME::COMMON::READER::MCL mcl;
		auto&& clustering = mcl.read_clustering(argv[4]);

		// Build name -> family map
		map<std::string, OrthologGroup*> families;
		for (auto&& family : database.get_ortholog_groups()) {
			assert(!family.is_merged());
			assert(family.size()>0);
			families[(*boost::begin(family.get_external_ids())).get_id()] = &family;
		}

		// Merge families according to clustering
		for (auto&& cluster : clustering) {
			OrthologGroup* first = nullptr;
			for (auto&& family_name : cluster) {
				auto&& family = *families.at(family_name);
				if (!first) {
					first = &family;
				}
				else {
					first->merge(std::move(family), database);
				}
			}
		}

		// Load itag -> pgsc mapping
		map<Gene*, Gene*> itag_to_pgsc;
		read_file(argv[5], [&database, &itag_to_pgsc](const char* begin, const char* end) { // TODO copy paste for the win
			using namespace boost::spirit::qi;

			auto on_line = [&database, &itag_to_pgsc](const std::vector<std::string>& line) {
				ensure(line.size() == 2,
						(make_string() << "Bogus row, expected col count == 2").str()
				);
				auto itag = database.try_get_gene(line.at(1));
				auto pgsc = database.try_get_gene(line.at(0));
				if (itag && pgsc) {
					itag_to_pgsc[itag] = pgsc;
				}
			};

			TabGrammarRules rules(true);
			parse(begin, end, rules.line[on_line] % eol);
			return begin;
		});

		// For each ITAG gene in family, add the (hopefully) equivalent PGSC gene
		for (auto&& family : database.get_ortholog_groups()) {
			flat_set<Gene*> pgsc_genes;
			for (auto&& gene : family.get_genes()) {
				if (contains(itag_to_pgsc, gene)) {
					pgsc_genes.insert(itag_to_pgsc.at(gene));
				}
			}
			for (auto&& gene : pgsc_genes) {
				family.add(*gene);
			}
		}

		// Print family sizes
		cout << "Families (after): " << boost::distance(database.get_ortholog_groups()) << endl;
		ofstream out("family_sizes.stat");
		for (auto&& family : database.get_ortholog_groups()) {
			assert(family.size() > 0);
			out << family.size() << " ";
		}

		// Save work
		for (auto&& group : database.get_gene_variant("AT1G02660").as_gene().get_ortholog_groups()) {
			cout << (uint64_t)group << endl;
		}
		database.save();
	});
}
