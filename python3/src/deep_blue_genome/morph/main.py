# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Blue Genome.
# 
# Deep Blue Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Blue Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.

from deep_blue_genome.core.reader.various import read_baits_file, read_expression_matrix_file,\
    read_gene_families_file
import sys
import pandas as pd
from deep_blue_genome.morph.algorithm import morph
from deep_blue_genome.core.cli import ArgumentParser

'''
'MOdule guided Ranking of candidate PatHway genes (MORPH)'
'''

def main():
    main_(sys.argv)

def main_(argv):
    def TopK(value):
        ivalue = int(value)
        if ivalue < 1:
             raise argparse.ArgumentTypeError("must provide an integer greater 0, got: %s".format(value))
        return ivalue

    # Parse CLI args
    parser = ArgumentParser(description='MOdule guided Ranking of candidate PatHway genes (MORPH).')
    parser.add_argument(
        '--baits-file', metavar='B', required=True, nargs='+',
        help='path to file listing the bait genes to use'
    )
    parser.add_argument(
        '--top-k', metavar='K', default=100, type=TopK,
        help='K best candidate genes to output in ranking'
    )
    args = parser.parse_args(argv[1:])

    # Read files
    baits = read_baits_file(args.baits_file)
    
    # Run alg
    network = morph(baits, args.top_k)

    # Write ranking to file
    CytoscapeWriter(network).write()

if __name__ == '__main__':
    main()
    
    
    
