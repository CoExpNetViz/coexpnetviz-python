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
import csv
import json
import logging
import sys

from varbio import (
    ExpressionMatrix, parse_baits, parse_csv, init_logging, UserError,
    join_lines
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

    def run(self):
        _init()
        self._parse_input()
        network = create_network(
            self._baits,
            self._expression_matrices,
            self._gene_families,
            self._percentile_ranks,
        )
        _print_json_response(network)
        self._write_sample_graphs(network)

    def _parse_input(self):
        args = json.load(sys.stdin)

        self._output_dir = Path(args['output_dir'])
        log_file = self._output_dir / 'coexpnetviz.log'
        init_logging('coexpnetviz', __version__, log_file)

        self._baits = _parse_json_baits(args)

        # If file names are not unique across matrices, it's up to the user to
        # rename them to be unique
        # TODO support non-csv formats too
        self._expression_matrices = []
        for matrix in args['expression_matrices']:
            matrix = Path(matrix)
            matrix = ExpressionMatrix.from_csv(matrix.name, parse_csv(matrix))
            self._expression_matrices.append(matrix)

        gene_families = args.get('gene_families', None)
        if gene_families:
            self._gene_families = parse_gene_families(Path(gene_families))
        else:
            self._gene_families = pd.DataFrame(columns=('family', 'gene'))

        self._percentile_ranks = _parse_percentile_ranks(args)
        logging.info(f'percentile ranks: {self._percentile_ranks}')

    def _write_sample_graphs(self, network):
        for info in network.matrix_infos:
            name = info.matrix.name
            sample_size = len(info.sample.index)

            flat_sample = info.sample.values.copy()
            # Ignore self correlations
            np.fill_diagonal(flat_sample, np.nan)
            # Flatten to a 1D array
            flat_sample = flat_sample[~np.isnan(flat_sample)].ravel()

            _write_sample_histogram(
                name, flat_sample, sample_size, self._output_dir, info.percentiles
            )
            _write_sample_cdf(
                name, flat_sample, sample_size, self._output_dir, self._percentile_ranks
            )

def _init():
    # TODO this is a quick and dirty fix to allow large csv files
    # https://stackoverflow.com/questions/15063936/csv-error-field-larger-than-field-limit-131072#15063941
    csv.field_size_limit(sys.maxsize)

    # Init matplotlib: the default backend does not work on a headless server
    # or on mac, Agg seems to work anywhere so use that instead, always.
    matplotlib.use('Agg')

def _parse_json_baits(args):
    baits = args['baits']
    min_baits = 2
    if isinstance(baits, str):
        baits = parse_baits(Path(baits), min_baits)
    else:
        assert isinstance(baits, list)
        if len(baits) < min_baits:
            raise UserError(f'Need at least 2 baits, but got only {len(baits)}')
    return pd.Series(baits)

def _parse_percentile_ranks(args):
    lower_rank = args['lower_percentile_rank']
    upper_rank = args['upper_percentile_rank']
    if lower_rank < 0:
        raise UserError(
            f'Lower percentile rank must be at least 0. Got: {lower_rank}'
        )
    if upper_rank > 100:
        raise UserError(
            f'Upper percentile rank must be at most 100. Got: {upper_rank}'
        )
    if lower_rank > upper_rank:
        raise ValueError(join_lines(
            f'''
            Lower percentile rank must be less or equal to upper percentile
            rank, got: {lower_rank}, {upper_rank}
            '''
        ))
    return np.array([lower_rank, upper_rank])

def _print_json_response(network):
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
        info.matrix.name: {
            'sample': info.sample.to_dict('records'),
            'cor_matrix': info.cor_matrix.to_dict('records'),
        }
        for info in network.matrix_infos
    }

    json.dump(response, sys.stdout)

def _write_sample_histogram(name, flat_sample, sample_size, output_dir, percentiles):
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
    plt.savefig(str(output_dir / f'{name}.sample_histogram.png'))

def _write_sample_cdf(name, flat_sample, sample_size, output_dir, percentile_ranks):
    plt.clf()
    pd.Series(flat_sample).plot.hist(bins=60, cumulative=True, density=True)
    plt.title(
        f'Cumulative distribution of correlations\n'
        f'between sample of {sample_size} genes in '
        f'{name}'
    )
    plt.xlabel('pearson')
    plt.ylabel('Cumulative probability, i.e. $P(cor \\leq x)$')
    plt.axhline(percentile_ranks[0]/100.0, **_line_style)
    plt.axhline(percentile_ranks[1]/100.0, **_line_style)
    plt.savefig(str(output_dir / f'{name}.sample_cdf.png'))

def main():
    App().run()

if __name__ == '__main__':
    # pylint does not understand click's magic
    # pylint: disable=no-value-for-parameter
    main()
