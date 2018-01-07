# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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
Test coexpnetviz.parse
'''

from coexpnetviz import parse
from pytil import data_frame as df_
from pytil.test import assert_text_contains
from textwrap import dedent
import pandas as pd
import numpy as np
import pytest

class TestValidateGeneFamilies(object):

    '''
    Test parse._validate_gene_families

    This is assumed to be called by parse.gene_families
    '''

    def test_happy_days(self):
        '''
        When valid gene families, do not raise
        '''
        original = pd.DataFrame(
            [
                ['fam1', 'geneA1'],
                ['fam1', 'geneA3'],
                ['fam2', 'geneA2'],
                ['fam2', 'geneB3'],
            ],
            columns=['family', 'gene']
        )
        df = original.copy()
        parse._validate_gene_families(df)
        df_.assert_equals(df, original)  # did not modify input

    parameters = {
        # When families overlap (before gene mapping), raise ValueError
        'overlap': (
            [
                ['fam1', 'gene1'],
                ['fam1', 'gene2'],
                ['fam2', 'gene2']
            ],
            '''\
                Gene families overlap:
                family   gene
                 fam1  gene2
                 fam2  gene2
            '''
        ),

        # When family name is not a str, raise ValueError
        'not_str': (
            [
                [None, 'gene1'],
                [np.nan, 'gene2'],
                [1, 'gene3'],
                ['ok', 'gene4']
            ],
            '''\
                Gene family names must be strings. Got:
                family     gene
                 None  'gene1'
                  nan  'gene2'
                    1  'gene3'
            '''
        ),

        # When family name is empty, raise ValueError
        'empty': (
            [
                ['', 'gene1'],
                ['hok', 'gene2'],
                ['ok', 'gene3']
            ],
            '''\
                Gene family names must not be empty. Got:
                family     gene
                   ''  'gene1'
            '''
        ),

        # When family name contains \0, raise ValueError
        'contains_null': (
            [
                ['har', 'gene1'],
                ['h\0k', 'gene2'],
                ['ok', 'gene3']
            ],
            '''\
                Gene family names must not contain a null character (\\x00). Got:
                family     gene
                'h\\x00k'  'gene2'
            '''
        )
    }
    @pytest.mark.parametrize('families, error', tuple(parameters.values()), ids=tuple(parameters.keys()))
    def test_raises(self, families, error):
        '''
        When invalid, raise ValueError
        '''
        with pytest.raises(ValueError) as ex:
            parse._validate_gene_families(pd.DataFrame(
                families, columns=['family', 'gene']
            ))
        assert_text_contains(ex.value.args[0], dedent(error.rstrip()))
