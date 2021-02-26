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
Test main.py

Everything other than matrix validation is either too trivial to test, very
obvious or not important if it does fail, or tricky to test automatically (such
as graph output).
'''

from pathlib import Path
from textwrap import dedent
import io
import json

from pytil.data_frame import assert_df_equals
from varbio import ExpressionMatrix, UserError
import numpy as np
import pandas as pd
import pytest

from coexpnetviz.main import main, _validate_matrices


class TestHappyDays:

    '''
    Test the most common scenario

    This is mainly a shotgun test to spot simple mistakes such as typos
    overlooked by the linter. This avoids distributing a version which would
    break for nearly 100% of all users/inputs. The aim isn't to check the
    final numbers are correct, as we already tested that in test_algorithm but
    just copy pasting the numbers was a quick but somewhat dirty alternative to
    mocking the create_network return.
    '''

    @pytest.fixture
    def matrix1(self, temp_dir_cwd):
        path = Path('matrix1')
        path.write_text(dedent('''\
            mygene\tcondition1\tcondition2\tcondition3
            gene1\t1\t2\t3
            gene2\t3\t2\t1
            gene3\t1\t2\t1
            gene4\t8\t3\t5'''
        ))
        return path

    @pytest.fixture
    def gene_families1(self, temp_dir_cwd):
        path = Path('gene_families1')
        path.write_text('fam1: [gene3, gene4]')
        return path

    @pytest.fixture
    def baits1(self, temp_dir_cwd):
        path = Path('baits1')
        path.write_text('gene1 gene2')
        return path

    @pytest.fixture
    def output_dir(self, temp_dir_cwd):
        output_dir = Path('output')
        output_dir.mkdir()
        return output_dir

    @pytest.fixture
    def mock_stdin(self, baits1, matrix1, gene_families1,
                   monkeypatch, output_dir):
        args = {
            'expression_matrices': [str(matrix1)],
            'baits': str(baits1),
            'gene_families': str(gene_families1),
            'output_dir': str(output_dir),
            'lower_percentile_rank': 5,
            'upper_percentile_rank': 95,
        }
        monkeypatch.setattr('sys.stdin', io.StringIO(json.dumps(args)))

    def test(self, mock_stdin, output_dir):
        main()

        # Sample matrix file
        expected = pd.DataFrame(
            [
                [1, -1, 0, -0.59603956067926978],
                [-1, 1, 0, 0.59603956067926978],
                [0, 0, 1, -0.802955],
                [-0.59603956067926978, 0.59603956067926978, -0.802955, 1]
            ],
            index=['gene1', 'gene2', 'gene3', 'gene4'],
            columns=['gene1', 'gene2', 'gene3', 'gene4'],
        )
        actual = pd.read_table(str(output_dir / 'matrix1.sample_matrix.txt'), index_col=0)
        assert_df_equals(actual, expected, ignore_order={0,1}, all_close=True)

        # Correlation matrix file
        expected = pd.DataFrame(
            [
                [1, -1],
                [-1, 1],
                [0, 0],
                [-0.59603956067926978, 0.59603956067926978]
            ],
            index=['gene1', 'gene2', 'gene3', 'gene4'],
            columns=['gene1', 'gene2'],
        )
        actual = pd.read_table(str(output_dir / 'matrix1.correlation_matrix.txt'), index_col=0)
        assert_df_equals(actual, expected, ignore_order={0,1}, all_close=True)

        # Percentiles file
        expected = pd.DataFrame(
            [
                ['matrix1', -0.95073877, 0.44702967]
            ],
            columns=['expression_matrix', 'lower', 'upper'],
        )
        actual = pd.read_table(str(output_dir / 'percentiles.txt'), index_col=None)
        assert_df_equals(actual, expected, ignore_order={0,1}, ignore_indices={0}, all_close=True)

        # Significant correlations file
        expected = pd.DataFrame(
            [
                ['gene1', 'gene2', -1],
                ['gene2', 'gene4', 0.59603956067926978]
            ],
            columns=['bait', 'gene', 'correlation'],
        )
        actual = pd.read_table(str(output_dir / 'significant_correlations.txt'), index_col=None)
        assert_df_equals(actual, expected, ignore_order={0,1}, ignore_indices={0}, all_close=True)

        # Sample graphs
        for file_name in ('matrix1.sample_histogram.png', 'matrix1.sample_cdf.png'):
            assert (output_dir / file_name).exists()

class TestValidateMatrices:

    def create_matrix(self, name, index):
        return ExpressionMatrix(
            name=name,
            data=pd.DataFrame(
                np.arange(len(index)).reshape(len(index), 1),
                index=index,
            )
        )

    @pytest.fixture
    def matrix_bait1(self):
        return self.create_matrix('mat1', ['bait1', 'gene1'])

    @pytest.fixture
    def matrix_bait1b(self):
        return self.create_matrix('mat2', ['bait1'])

    @pytest.fixture
    def matrix_gene1b(self):
        return self.create_matrix('mat3', ['bait2', 'gene1'])

    @pytest.fixture
    def baits0(self):
        return pd.Series([], dtype='object')

    @pytest.fixture
    def baits1(self):
        return pd.Series(['bait1'])

    @pytest.fixture
    def baits2(self):
        return pd.Series(['bait1', 'bait2'])

    def test_raise_if_unused_bait(self, baits2, matrix_bait1):
        # Given bait2 does not appear in any matrix
        with pytest.raises(UserError) as ex:
            _validate_matrices(baits2, [matrix_bait1])
        assert 'baits is either missing from all or present' in str(ex.value)

    def test_raise_if_overlapping_bait(self, baits1, matrix_bait1, matrix_bait1b):
        # Given bait1 appears in multiple matrices
        with pytest.raises(UserError) as ex:
            _validate_matrices(baits1, [matrix_bait1, matrix_bait1b])
        assert 'baits is either missing from all or present' in str(ex.value)

    def test_raise_if_overlapping_gene(self, baits2, matrix_bait1, matrix_gene1b):
        # Given gene1 (non-bait) appears in multiple matrices
        with pytest.raises(UserError) as ex:
            _validate_matrices(baits2, [matrix_bait1, matrix_gene1b])
        assert 'genes appear in multiple' in str(ex.value)

    def test_raise_if_baitless_matrix(self, baits0, matrix_bait1):
        # Given a matrix without any baits
        with pytest.raises(UserError) as ex:
            _validate_matrices(baits0, [matrix_bait1])
        assert 'matrices have no baits' in str(ex.value)
