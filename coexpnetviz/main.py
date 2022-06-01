# Copyright (C) 2015-2021 VIB/BEG/UGent - Tim Diels <tim@diels.me>
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

# Init matplotlib: the default backend does not work on a headless server
# or on mac, Agg seems to work anywhere so use that instead, always.
# Best to do this as early as possible, e.g. pyplot import already loads the
# backend.
#
# This turns imports into a mess, so we must pylint ignore it
# pylint: disable=wrong-import-order,wrong-import-position,ungrouped-imports
import matplotlib
matplotlib.use('Agg')


from pathlib import Path
from itertools import product
from textwrap import dedent
import csv
import ctypes
import json
import logging
import sys

from varbio import (
    ExpressionMatrix, parse_baits, parse_csv, UserError,
    join_lines
)
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from coexpnetviz import __version__
from coexpnetviz._algorithm import create_network
from coexpnetviz._various import parse_gene_families


_line_style = {'color': 'r', 'linewidth': 2}


class App:

    def run(self):
        try:
            _init()
            self._parse_input()
            network = create_network(
                self._baits,
                self._expression_matrices,
                self._gene_families,
                self._percentiles,
            )
            _print_json_response(network)
            self._write_sample_graphs(network)
            _write_matrix_intermediates(network, self._output_dir)
            _write_percentile_values(network, self._output_dir)
            _write_significant_cors(network, self._output_dir)
        except BrokenPipeError:
            # Broken pipe error tends to happen when our Cytoscape app stops
            # reading stdout/stderr. Sometimes this exits as 1, sometimes as
            # 120; here we ensure it always uses 120. This way the Cytoscape
            # app knows to expect a read/parse error on its own end rather than
            # showing the broken pipe error.
            #
            # Do not attempt to log here as we log to stderr which probably
            # broke as well.
            sys.exit(120)

    def _parse_input(self):
        with open(sys.argv[1]) as f:
            args = json.load(f)

        self._output_dir = Path(args['output_dir'])
        log_file = self._output_dir / 'coexpnetviz.log'
        _init_logging(log_file)

        self._baits = _parse_json_baits(args)

        # If file names are not unique across matrices, it's up to the user to
        # rename them to be unique
        # TODO support non-csv formats too
        self._expression_matrices = []
        for matrix in args['expression_matrices']:
            matrix = Path(matrix)
            matrix = ExpressionMatrix.from_csv(matrix.name, parse_csv(matrix))
            self._expression_matrices.append(matrix)
        _validate_matrices(self._baits, self._expression_matrices)

        gene_families = args.get('gene_families', None)
        if gene_families:
            self._gene_families = parse_gene_families(Path(gene_families))
        else:
            self._gene_families = pd.DataFrame(columns=('family', 'gene'))

        self._percentiles = _parse_percentiles(args)
        logging.info(f'percentiles: {self._percentiles}')

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
                name, flat_sample, sample_size, self._output_dir, info.percentile_values
            )
            _write_sample_cdf(
                name, flat_sample, sample_size, self._output_dir, self._percentiles
            )


def _init():
    # Avoid any csv field size limit errors by setting it to the max value for
    # the current architecture. This does not affect any buffers CPython
    # allocates, it's just a validation check. The magic below is explained by
    # https://stackoverflow.com/a/54517228
    csv.field_size_limit(int(ctypes.c_ulong(-1).value // 2))

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

def _parse_percentiles(args):
    lower_percentile = args['lower_percentile']
    upper_percentile = args['upper_percentile']
    if lower_percentile < 0:
        raise UserError(
            f'Lower percentile must be at least 0. Got: {lower_percentile}'
        )
    if upper_percentile > 100:
        raise UserError(
            f'Upper percentile must be at most 100. Got: {upper_percentile}'
        )
    if lower_percentile > upper_percentile:
        raise ValueError(join_lines(
            f'''
            Lower percentile must be less or equal to upper percentile, got:
            {lower_percentile}, {upper_percentile}
            '''
        ))
    return np.array([lower_percentile, upper_percentile])

def _validate_matrices(baits, matrices):
    if not matrices:
        raise UserError(join_lines(
            f'''
            Must provide at least one expression matrix, got:
            {matrices}
            '''
        ))
    names = [matrix.name for matrix in matrices]
    if len(matrices) != len(set(names)):
        raise UserError(
            f'Expression matrices must have unique name, got: {sorted(names)}'
        )

    # Check each bait occurs in exactly one matrix
    bait_presence = np.array([
        bait in matrix.data.index
        for matrix, bait in product(matrices, baits)
    ])
    bait_presence = bait_presence.reshape(len(matrices), len(baits))
    missing_bait_matrix = pd.DataFrame(
        bait_presence,
        index=matrices,
        columns=baits
    )
    missing_bait_matrix = missing_bait_matrix.loc[:,bait_presence.sum(axis=0) != 1]
    # pylint: disable=trailing-whitespace
    if not missing_bait_matrix.empty:
        missing_bait_matrix = missing_bait_matrix.applymap(lambda x: 'present' if x else 'absent')
        missing_bait_matrix.index = missing_bait_matrix.index.map(lambda matrix: matrix.name)
        missing_bait_matrix.index.name = 'Matrix name'
        missing_bait_matrix.columns.name = 'Gene name'
        raise UserError(dedent(
            f'''\
            Each of the following baits is either missing from all or present in
            multiple expression matrices:
            
            {missing_bait_matrix.to_string()}
            
            Missing baits are columns with no "present" value, while baits in
            multiple matrices have multiple "present" values in a column.'''
        ))

    # and each matrix has at least one bait
    is_baitless = bait_presence.sum(axis=1) == 0
    if is_baitless.any():
        matrices = np.array(matrices)[is_baitless]
        matrices = ', '.join(map(str, matrices))
        raise UserError(join_lines(
            f'''
            Some expression matrices have no baits: {matrices}. Each expression
            matrix must contain at least one bait. Either drop the matrices or
            add some of their genes to the baits list.
            '''
        ))

    # Check the matrices don't overlap (same gene in multiple matrices)
    all_genes = pd.Series(sum((list(matrix.data.index) for matrix in matrices), []))
    overlapping_genes = all_genes[all_genes.duplicated()]
    if not overlapping_genes.empty:
        raise UserError(join_lines(
            f'''
            The following genes appear in multiple expression matrices:
            {', '.join(overlapping_genes)}. CoExpNetViz does not support gene
            expression data from different matrices for the same gene. Please
            remove rows from the given matrices such that no gene appears in
            multiple matrices.
            '''
        ))

def _print_json_response(network):
    response = {}

    nodes = network.nodes.copy()
    nodes['colour'] = nodes['colour'].apply(lambda x: x.to_hex())
    # convert frozenset to tuple as json.dump does not support frozenset
    nodes['genes'] = nodes['genes'].apply(tuple)
    response['nodes'] = nodes.to_dict('records')

    response['homology_edges'] = network.homology_edges.to_dict('records')
    response['cor_edges'] = network.cor_edges.to_dict('records')

    json.dump(response, sys.stdout)

def _write_sample_histogram(name, flat_sample, sample_size, output_dir, percentile_values):
    plt.clf()
    pd.Series(flat_sample).plot.hist(bins=60)
    plt.title(
        f'Correlations between sample of\n'
        f'{sample_size} genes in {name}'
    )
    plt.xlabel('pearson')
    plt.ylabel('frequency')
    plt.axvline(percentile_values[0], **_line_style)
    plt.axvline(percentile_values[1], **_line_style)
    plt.savefig(str(output_dir / f'{name}.sample_histogram.png'))

def _write_sample_cdf(name, flat_sample, sample_size, output_dir, percentiles):
    plt.clf()
    pd.Series(flat_sample).plot.hist(bins=60, cumulative=True, density=True)
    plt.title(
        f'Cumulative distribution of correlations\n'
        f'between sample of {sample_size} genes in '
        f'{name}'
    )
    plt.xlabel('pearson')
    plt.ylabel('Cumulative probability, i.e. $P(cor \\leq x)$')
    plt.axhline(percentiles[0]/100.0, **_line_style)
    plt.axhline(percentiles[1]/100.0, **_line_style)
    plt.savefig(str(output_dir / f'{name}.sample_cdf.png'))

def _write_matrix_intermediates(network, output_dir):
    for info in network.matrix_infos:
        name = info.matrix.name

        sample = info.sample
        sample.index.name = None
        sample_file = str(output_dir / f'{name}.sample_matrix.txt')
        sample.to_csv(sample_file, sep='\t', na_rep=str(np.nan))

        cor_matrix = info.cor_matrix
        cor_matrix.index.name = None
        cor_file = str(output_dir / f'{name}.correlation_matrix.txt')
        cor_matrix.to_csv(cor_file, sep='\t', na_rep=str(np.nan))

def _write_percentile_values(network, output_dir):
    data = tuple(
        (info.matrix.name,) + info.percentile_values
        for info in network.matrix_infos
    )
    percentile_values = pd.DataFrame(data, columns=('expression_matrix', 'lower', 'upper'))
    percentile_values_file = str(output_dir / 'percentile_values.txt')
    percentile_values.to_csv(percentile_values_file, sep='\t', na_rep=str(np.nan), index=False)

def _write_significant_cors(network, output_dir):
    output_file = output_dir / 'significant_correlations.txt'
    network.significant_cors.to_csv(
        str(output_file), sep='\t', na_rep=str(np.nan), index=False,
    )
def _init_logging(log_file):
    root_logger = logging.getLogger()
    root_logger.setLevel(logging.INFO)
    logging.getLogger('coexpnetviz').setLevel(logging.DEBUG)

    formatter = logging.Formatter('{asctime} {levelname[0]}: {message}', style='{')

    # Log to file
    file_handler = logging.FileHandler(str(log_file))
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    root_logger.addHandler(file_handler)

def main():
    try:
        App().run()
    except Exception:
        logging.exception('Uncaught exception')
        raise

if __name__ == '__main__':
    # pylint does not understand click's magic
    # pylint: disable=no-value-for-parameter
    main()
