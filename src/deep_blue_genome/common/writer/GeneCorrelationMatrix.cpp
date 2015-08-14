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

#include <deep_blue_genome/common/stdafx.h>
#include "GeneCorrelationMatrix.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
#include <deep_blue_genome/util/printer.h>

using namespace DEEP_BLUE_GENOME;
using namespace std;
using boost::irange;
using boost::adaptors::transformed;

namespace DEEP_BLUE_GENOME {
namespace COMMON {
namespace WRITER {

std::string write_plain(const GeneExpressionMatrix& expression_matrix, const GeneCorrelationMatrix& correlation_matrix) {
	using namespace std::placeholders;

	// Note: corr mat and exp mat row indices refer to the same genes
	auto&& matrix = correlation_matrix.get();

	// Format header
	auto&& header = make_printer([&expression_matrix, &correlation_matrix, &matrix](std::ostream& out) {
		out << "\t" << intercalate("\t",
			irange(static_cast<decltype(matrix.size2())>(0), matrix.size2())
				| transformed(bind(&GeneCorrelationMatrix::get_row_index, &correlation_matrix, _1))
				| transformed(bind(&GeneExpressionMatrix::get_gene, &expression_matrix, _1))
				| transformed(bind(&Gene::get_name, _1))
		);
	});

	// Format lines
	auto&& lines = make_printer([&expression_matrix, &correlation_matrix, &matrix](std::ostream& out){
		for (decltype(matrix.size1()) row=0; row<matrix.size1(); row++) {
			assert(expression_matrix.get().size1() == matrix.size1());
			out << expression_matrix.get_gene(row).get_name();
			for (decltype(matrix.size2()) col=0; col < matrix.size2(); col++) {
				out << "\t" << matrix(row, col);
			}
			out << "\n";
		}
	});

	return intercalate_("\n", header, lines);
}

}}} // end namespace
