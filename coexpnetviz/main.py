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
from pkg_resources import resource_string  # @UnresolvedImport
import argparse
import csv
import logging
import sys

from varbio import (
    ExpressionMatrix, parse_baits, parse_csv, parse_yaml, init_logging
)
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from coexpnetviz import (
    __version__, write_cytoscape, create_network, parse_gene_families
)


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
        self._network = create_network(
            self._baits,
            self._expression_matrices,
            self._gene_families,
            self._percentile_ranks,
        )
        write_cytoscape(self._network, 'network', self._output_dir)
        self._write_matrix_intermediates()
        self._write_percentiles()
        self._network.significant_correlations.to_csv(
            str(self._output_dir / 'significant_correlations.txt'),
            sep='\t',
            na_rep=str(np.nan),
            index=False,
        )
        (self._output_dir / 'README.txt').write_bytes(resource_string(__name__, 'data/README.txt'))

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

    def _write_matrix_intermediates(self):
        matrix_outputs = zip(
            self._expression_matrices,
            self._network.samples,
            self._network.correlation_matrices,
            self._network.percentiles,
        )
        for expression_matrix, sample, correlation_matrix, percentiles in matrix_outputs:
            name = expression_matrix.name

            sample.index.name = None
            sample_file = str(self._output_dir / '{}.sample_matrix.txt'.format(name))
            sample.to_csv(sample_file, sep='\t', na_rep=str(np.nan))

            correlation_matrix.index.name = None
            correlation_file = str(self._output_dir / '{}.correlation_matrix.txt'.format(name))
            correlation_matrix.to_csv(correlation_file, sep='\t', na_rep=str(np.nan))

            # Flatten sample matrix
            sample_size = len(sample.index)
            sample = sample.values.copy()
            np.fill_diagonal(sample, np.nan)
            sample = sample[~np.isnan(sample)].ravel()

            # Write histogram
            line_style = dict(color='r', linewidth=2)
            plt.clf()
            pd.Series(sample).plot.hist(bins=60)
            plt.title(f'Correlations between sample of\n{sample_size} genes in {expression_matrix.name}')
            plt.xlabel('pearson')
            plt.ylabel('frequency')
            plt.axvline(percentiles[0], **line_style)
            plt.axvline(percentiles[1], **line_style)
            plt.savefig(str(self._output_dir / f'{expression_matrix.name}.sample_histogram.png'))

            # Write cdf
            plt.clf()
            pd.Series(sample).plot.hist(bins=60, cumulative=True, density=True)
            plt.title(f'Cumulative distribution of correlations\nbetween sample of {sample_size} genes in {expression_matrix.name}')
            plt.xlabel('pearson')
            plt.ylabel('Cumulative probability, i.e. $P(corr \\leq x)$')
            plt.axhline(self._percentile_ranks[0]/100.0, **line_style)
            plt.axhline(self._percentile_ranks[1]/100.0, **line_style)
            plt.savefig(str(self._output_dir / f'{expression_matrix.name}.sample_cdf.png'))

    def _write_percentiles(self):
        percentiles = pd.DataFrame(self._network.percentiles, columns=('lower', 'upper'))
        percentiles.insert(0, 'expression_matrix', self._expression_matrices)
        percentiles['expression_matrix'] = (
            percentiles['expression_matrix'].apply(lambda matrix: matrix.name)
        )
        percentiles_file = str(self._output_dir / 'percentiles.txt')
        percentiles.to_csv(percentiles_file, sep='\t', na_rep=str(np.nan), index=False)

def main():
    App().run()

if __name__ == '__main__':
    # pylint does not understand click's magic
    # pylint: disable=no-value-for-parameter
    main()
