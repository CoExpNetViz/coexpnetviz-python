# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <tim@diels.me>
#
# This file is part of CoExpNetViz.
#
# CoExpNetViz is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CoExpNetViz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with CoExpNetViz.  If not, see <http://www.gnu.org/licenses/>.

from pathlib import Path
import argparse
import csv
import json
import logging
import sys

from varbio import (
    ExpressionMatrix, parse_baits, parse_csv, parse_yaml, init_logging
)
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from coexpnetviz import __version__
from coexpnetviz._algorithm import create_network
from coexpnetviz._various import parse_gene_families


_line_style = {'color': 'r', 'linewidth': 2}

class App:

    '''
    Comparative Co-Expression Network Construction and Visualization
    (CoExpNetViz)

    Accept a single CLI arg, path to a yaml config file:

        baits: path/to/baits.txt

        # 1 or more expression matrices
        expression_matrices:
          - path/to/expmat1.csv
          - path/to/expmat2.xlsx

        # All output files will be placed in this dir
        output_dir: path/to/output_dir

        # Which percentile rank to use to determine the lower and upper cut-off. For
        # each expression matrix, a sample of genes is drawn and the correlations
        # between them is calculated. From this sample distribution of correlations
        # the L-th percentile is used as a cut-off to determine whether 2 genes in the
        # matrix are co-expressed or not.
        percentile_ranks: [5.0, 95.0]

        # Optionally
        gene_families: path/to/gene_families.yml
    '''

    def run(self):
        self._init()
        config_file = self._parse_args()
        self._parse_config(config_file)
        network = create_network(
            self._baits,
            self._expression_matrices,
            self._gene_families,
            self._percentile_ranks,
        )
        self._print_json_response(network)

    @staticmethod
    def _init():
        # TODO this is a quick and dirty fix to allow large csv files
        # https://stackoverflow.com/questions/15063936/csv-error-field-larger-than-field-limit-131072#15063941
        csv.field_size_limit(sys.maxsize)

        # Init matplotlib: the default backend does not work on a headless server
        # or on mac, Agg seems to work anywhere so use that instead, always.
        matplotlib.use('Agg')

    @staticmethod
    def _parse_args():
        parser = argparse.ArgumentParser(
            description='Create a CoExpNetViz network from baits and matrices'
        )
        parser.add_argument(
            'config_file',
            help='A yaml config file containing input and parameters',
        )
        args = parser.parse_args()
        return Path(args.config_file)

    def _parse_config(self, path):
        config = parse_yaml(path)

        self._output_dir = Path(config['output_dir'])
        log_file = self._output_dir / 'coexpnetviz.log'
        init_logging('coexpnetviz', __version__, log_file)

        baits = Path(config['baits'])
        self._baits = pd.Series(parse_baits(baits, min_baits=2))

        # If file names are not unique across matrices, it's up to the user to
        # rename them to be unique
        self._expression_matrices = []
        for matrix in config['expression_matrices']:
            matrix = Path(matrix)
            matrix = ExpressionMatrix.from_csv(matrix.name, parse_csv(matrix))
            self._expression_matrices.append(matrix)

        gene_families = config.get('gene_families', None)
        if gene_families:
            self._gene_families = parse_gene_families(Path(gene_families))
        else:
            self._gene_families = pd.DataFrame(columns=('family', 'gene'))

        self._percentile_ranks = config['percentile_ranks']
        logging.info(f'percentile ranks: {self._percentile_ranks}')

    def _print_json_response(self, network):
        response = {}

        nodes = network.nodes.copy()
        nodes['colour'] = nodes['colour'].apply(lambda x: x.to_hex())
        response['nodes'] = nodes.to_dict('records')

        data = tuple(
            (info.matrix.name,) + info.percentiles
            for info in network.matrix_infos
        )
        percentiles = pd.DataFrame(data, columns=('expression_matrix', 'lower', 'upper'))
        response['percentiles'] = percentiles.to_dict('records')

        # TODO test empty dfs
        response['homology_edges'] = network.homology_edges.to_dict('records')
        response['cor_edges'] = network.cor_edges.to_dict('records')
        response['significant_cors'] = network.significant_cors.to_dict('records')
        response['matrix_infos'] = {
            info.matrix.name: self._dump_matrix_info(info)
            for info in network.matrix_infos
        }

        json.dump(response, sys.stdout)

    def _dump_matrix_info(self, info):
        response = {}
        name = info.matrix.name
        response['sample'] = info.sample.to_dict('records')
        response['cor_matrix'] = info.cor_matrix.to_dict('records')

        # Also output 2 graphs about the sample to file
        sample_size = len(info.sample.index)
        flat_sample = info.sample.values.copy()
        np.fill_diagonal(flat_sample, np.nan)
        flat_sample = flat_sample[~np.isnan(flat_sample)].ravel()
        self._write_sample_histogram(
            name, flat_sample, sample_size, info.percentiles
        )
        self._write_sample_cdf(
            name, flat_sample, sample_size
        )

    def _write_sample_histogram(self, name, flat_sample, sample_size, percentiles):
        plt.clf()
        pd.Series(flat_sample).plot.hist(bins=60)
        plt.title(
            f'Correlations between sample of\n'
            f'{sample_size} genes in {name}'
        )
        plt.xlabel('pearson')
        plt.ylabel('frequency')
        plt.axvline(percentiles[0], **_line_style)
        plt.axvline(percentiles[1], **_line_style)
        plt.savefig(str(self._output_dir / f'{name}.sample_histogram.png'))

    def _write_sample_cdf(self, name, flat_sample, sample_size):
        plt.clf()
        pd.Series(flat_sample).plot.hist(bins=60, cumulative=True, density=True)
        plt.title(
            f'Cumulative distribution of correlations\n'
            f'between sample of {sample_size} genes in '
            f'{name}'
        )
        plt.xlabel('pearson')
        plt.ylabel('Cumulative probability, i.e. $P(cor \\leq x)$')
        plt.axhline(self._percentile_ranks[0]/100.0, **_line_style)
        plt.axhline(self._percentile_ranks[1]/100.0, **_line_style)
        plt.savefig(str(self._output_dir / f'{name}.sample_cdf.png'))

def main():
    App().run()

if __name__ == '__main__':
    # pylint does not understand click's magic
    # pylint: disable=no-value-for-parameter
    main()
