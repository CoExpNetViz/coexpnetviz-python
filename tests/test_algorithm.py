# Copyright (C) 2021 VIB/BEG/UGent - Tim Diels <tim@diels.me>
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

'''
Test _algorithm

Too trivial to test: _create_nodes.
'''

from unittest.mock import Mock
import pytest

from pytil.data_frame import assert_df_equals
from pytil.series import assert_equals as assert_series_equals
from varbio import ExpressionMatrix
import pandas as pd
import numpy as np

from coexpnetviz._various import RGB
import coexpnetviz._algorithm as alg


@pytest.fixture
def pearson_df_mock(monkeypatch):
    mock = Mock()
    monkeypatch.setattr('coexpnetviz._algorithm.pearson_df', mock)
    return mock

class TestEstimateCutoffs:

    '''
    Happy days scenario to spot stack traces and to ensure percentiles are
    correct. Manually reviewed the rest of it.
    '''

    @pytest.fixture
    def matrix(self):
        'A tiny matrix (<800 rows)'
        return ExpressionMatrix(
            name='mat',
            data=pd.DataFrame(np.arange(6).reshape(3, 2), dtype=float),
        )

    @pytest.fixture
    def expected_cors(self, pearson_df_mock):
        expected_sample = pd.DataFrame(
            [[1,  1, 2],
             [1,  1, 3],
             [2,  3, 1]],
            dtype=float
        )
        pearson_df_mock.return_value = expected_sample.copy()
        return expected_sample

    @pytest.fixture
    def percentile_mock(self, monkeypatch):
        mock = Mock()
        monkeypatch.setattr('numpy.percentile', mock)
        return mock

    def test(self, pearson_df_mock, matrix, expected_cors, percentile_mock):
        orig_matrix_df = matrix.data.copy()
        percentile_ranks = np.array([20.0, 80.0])
        orig_percentile_ranks = percentile_ranks.copy()
        cors, _ = alg._estimate_cutoffs(
            matrix, percentile_ranks=percentile_ranks
        )

        # Input is unchanged
        assert_df_equals(matrix.data, orig_matrix_df)
        assert np.allclose(percentile_ranks, orig_percentile_ranks)

        # Then correlations of the entire matrix are calculated (instead of just a
        # sample)
        args = pearson_df_mock.call_args.args
        assert_df_equals(args[0], matrix.data)
        assert_df_equals(args[1], matrix.data)

        # the actual sample cors are the one returned by pearson
        assert_df_equals(cors, expected_cors)

        # percentiles are calculated on a triangle minus the diagonal
        args = percentile_mock.call_args.args
        assert np.allclose(args[0], np.array([1.0, 2.0, 3.0]))
        assert np.allclose(args[1], percentile_ranks)

class TestCorrelateMatrix:

    '''
    Happy days scenario to check for errors, not dropping way too much due to
    buggy low std check, cutting off the right cors, outputting in the right
    formats.
    '''

    @pytest.fixture
    def percentiles(self):
        'Values which cut some but not all cors'
        return np.array([1.0, 5.0])

    @pytest.fixture
    def estimate_cutoffs_mock(self, monkeypatch, percentiles):
        # sample is a bogus value, but sufficient for this test
        mock = Mock(return_value=('sample', percentiles.copy()))
        monkeypatch.setattr('coexpnetviz._algorithm._estimate_cutoffs', mock)
        return mock

    @pytest.fixture
    def percentile_ranks(self):
        '''
        Some percentile ranks, values do not matter as we mock
        _estimate_cutoffs
        '''
        return np.array([1.0, 99.0])

    @pytest.fixture
    def matrix(self):
        'Matrix with plenty of std, some baits, some non-baits'
        return ExpressionMatrix(
            name='mat',
            data=pd.DataFrame(
                np.arange(6).reshape(3, 2),
                index=['bait1', 'bait2', 'gene1'],
                columns=['experiment1', 'experiment2'],
                dtype=float,
            ),
        )

    @pytest.fixture
    def baits(self):
        'Some baits to go with the matrix'
        return pd.Series(['bait1', 'bait2', 'bait_not_present'])

    @pytest.fixture
    def cor_matrix(self):
        return pd.DataFrame(
            np.arange(6).reshape(3, 2),
            index=['bait1', 'bait2', 'gene1'],
            columns=['bait1', 'bait2'],
            dtype=float,
        )

    @pytest.fixture
    def pearson_df_mock(self, pearson_df_mock, cor_matrix):
        pearson_df_mock.return_value = cor_matrix
        return pearson_df_mock

    def test(self, matrix, baits, pearson_df_mock, estimate_cutoffs_mock,
             percentile_ranks, cor_matrix, percentiles):

        orig_matrix_df = matrix.data.copy()
        orig_baits = baits.copy()
        orig_percentile_ranks = percentile_ranks.copy()
        cors, matrix_info = alg._correlate_matrix(
            matrix, baits, percentile_ranks
        )

        # Then input unchanged
        assert_df_equals(matrix.data, orig_matrix_df)
        assert_series_equals(baits, orig_baits)
        assert np.allclose(percentile_ranks, orig_percentile_ranks)

        # Correlate entire matrix (so no rows were dropped due to low std) to
        # present baits
        args = pearson_df_mock.call_args.args
        assert_df_equals(args[0], orig_matrix_df)
        assert list(args[1].index) == ['bait1', 'bait2']

        # Matrix info is passed on unchanged
        assert_df_equals(matrix_info.cor_matrix, cor_matrix)
        assert np.allclose(matrix_info.percentiles, percentiles)
        assert matrix_info.sample == 'sample'

        # Insignificant cors have been cut, but only those. And the cors df has
        # a different format.
        expected_cors = pd.DataFrame(
            [['bait1', 'bait1', 0.0],
             ['bait1', 'bait2', 1.0],
             ['gene1', 'bait2', 5.0]],
            columns=['gene', 'bait', 'correlation']
        )
        assert_df_equals(
            cors, expected_cors, ignore_indices={0}, ignore_order={0, 1}
        )

class TestCorrelateMatrices:

    '''
    Happy days, for code coverage, check self/symmetrical cors are dropped,
    concat correctly. Not checking for trivial things though, manually reviewed
    that instead.
    '''

    @pytest.fixture
    def correlate_matrix_mock(self, monkeypatch):
        cors1 = pd.DataFrame(
            [['bait1', 'gene1', 0.5],
             ['gene1', 'bait1', 0.5],
             ['bait1', 'bait1', 1.0]],
            columns=['bait', 'gene', 'correlation'],
        )
        cors2 = pd.DataFrame(
            [['bait2', 'gene2', 1.0],
             ['gene2', 'bait2', 1.0],
             ['bait2', 'bait2', 1.0]],
            columns=['bait', 'gene', 'correlation'],
        )
        mock = Mock(side_effect=((cors1, 3), (cors2, 4)))
        monkeypatch.setattr('coexpnetviz._algorithm._correlate_matrix', mock)
        return mock

    def test(self, correlate_matrix_mock):
        # These args are invalid but is fine for this test as we mock _correlate_matrix
        cors, matrix_infos = alg._correlate_matrices([1, 2], None, None)

        # Then cors is concatenation of the cors of each matrix with self
        # comparisons and symmetrical cors dropped (i.e. only return
        # pearson(x, y) for x < y)
        expected = pd.DataFrame(
            [['bait1', 'gene1', 0.5],
             ['bait2', 'gene2', 1.0]],
            columns=['bait', 'gene', 'correlation'],
        )
        assert_df_equals(cors, expected, ignore_indices={0}, ignore_order={0, 1})

        # matrix_infos is a tuple of infos (though we only returned an int
        # instead of MatrixInfo)
        assert matrix_infos == (3, 4)

class TestCreateBaitNodes:

    '''
    Happy days, for coverage and mostly to check the gene family is merged in
    correctly. The remainder is fairly trivial.
    '''

    @pytest.fixture
    def baits(self):
        return pd.Series(['bait1', 'bait2'])

    @pytest.fixture
    def gene_families(self):
        return pd.DataFrame(
            [['bait2', 'fam2']],
            columns=['gene', 'family'],
        )

    def test(self, baits, gene_families):
        orig_baits = baits.copy()
        orig_families = gene_families.copy()
        nodes = alg._create_bait_nodes(baits, gene_families)

        # Then input unchanged
        assert_series_equals(baits, orig_baits)
        assert_df_equals(gene_families, orig_families)

        # and returns a table with family added
        colour = RGB((255, 255, 255))
        partition = hash(frozenset())
        expected = pd.DataFrame(
            [[frozenset({'bait1'}), 'bait', np.nan, 'bait1', colour, partition],
             [frozenset({'bait2'}), 'bait', 'fam2', 'bait2', colour, partition]],
            columns=[
                'genes', 'type', 'family', 'label', 'colour', 'partition_id'
            ],
        )
        assert_df_equals(
            nodes, expected, ignore_indices={0}, ignore_order={0, 1}
        )

class TestCreateNonBaitNodes:

    '''
    Happy days, mostly check for a correct split between family/gene nodes,
    check families are merged in correctly and baits are ignored.
    '''

    @pytest.fixture
    def baits(self):
        return pd.Series(['bait1', 'bait2'])

    @pytest.fixture
    def gene_families(self):
        return pd.DataFrame(
            [['gene1', 'fam'],
             ['gene2', 'fam']],
            columns=['gene', 'family'],
        )

    @pytest.fixture
    def cors(self):
        return pd.DataFrame(
            [['bait1', 'gene1', 1.0],
             ['bait1', 'gene2', 2.0],
             ['bait2', 'gene1', 3.0],
             ['bait2', 'gene3', 4.0],
             ['bait1', 'bait2', 5.0]],
            columns=['bait', 'gene', 'correlation'],
        )

    def test(self, baits, cors, gene_families):
        orig_baits = baits.copy()
        orig_cors = cors.copy()
        orig_gene_families = gene_families.copy()
        family_nodes, gene_nodes = alg._create_non_bait_nodes(baits, cors, gene_families)

        # Then input unchanged
        assert_series_equals(baits, orig_baits)
        assert_df_equals(cors, orig_cors)
        assert_df_equals(gene_families, orig_gene_families)

        # gene nodes
        expected = pd.DataFrame(
            [[frozenset({'bait2'}), 'gene3', 'gene', frozenset({'gene3'})]],
            columns=[
                'baits', 'label', 'type', 'genes'
            ],
        )
        assert_df_equals(
            gene_nodes, expected, ignore_indices={0}, ignore_order={0, 1}
        )

        # family nodes
        expected = pd.DataFrame(
            [[
                frozenset({'bait1', 'bait2'}),
                'fam',
                'family',
                frozenset({'gene1', 'gene2'}),
                'fam'
            ]],
            columns=[
                'baits', 'label', 'type', 'genes', 'family'
            ],
        )
        assert_df_equals(
            family_nodes, expected, ignore_indices={0}, ignore_order={0, 1}
        )
