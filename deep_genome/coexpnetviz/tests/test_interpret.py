# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Genome.
# 
# Deep Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Genome.  If not, see <http://www.gnu.org/licenses/>.

'''
Test deep_genome.coexpnetviz._interpret
'''

from deep_genome.coexpnetviz import _interpret
from chicken_turtle_util import data_frame as df_
from textwrap import dedent
import pandas as pd
import numpy as np
import pytest
        
class TestGeneFamilies(object):
    
    '''
    Test _interpret.gene_families
    '''
    
    def test_happy_days(self, session):
        '''
        When valid input, return fine
        
        I.e. replace gene str with Gene objects 
        '''
        original = pd.DataFrame(
            [
                ['fam1', 'gene1'],
                ['fam1', 'gene2'],
                ['fam2', 'gene3'],
            ],
            columns=['family', 'gene']
        )
        df = original.copy()
        actual = _interpret.gene_families(session, df)
        df_.assert_equals(df, original)  # no edit original
        
        actual['gene'] = actual['gene'].apply(lambda gene: gene.name)
        df_.assert_equals(actual, original, ignore_order={0})
        
    def test_gene_mapping(self, session):
        '''
        When valid input and gene mapping that does not cause overlap, return fine
        
        I.e. apply mapping and return Gene's
        '''
        session.add_gene_mapping(pd.DataFrame(
            [
                ['geneA1', 'geneB1'],
                ['geneA1', 'geneB2'],
                ['geneA2', 'geneB3'],
            ],
            columns=['source', 'destination']
        ))
        
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
        actual = _interpret.gene_families(session, df)
        df_.assert_equals(df, original)  # no edit original
        
        actual['gene'] = actual['gene'].apply(lambda gene: gene.name)
        expected = pd.DataFrame(
            [
                ['fam1', 'geneB1'],
                ['fam1', 'geneB2'],
                ['fam1', 'geneA3'],
                ['fam2', 'geneB3'],
            ],
            columns=['family', 'gene']
        )
        df_.assert_equals(actual, expected, ignore_order={0})
    
    def test_empty(self, session):
        '''
        When given an empty DataFrame, return empty
        '''
        df = pd.DataFrame(columns=['family', 'gene'])
        actual = _interpret.gene_families(session, df)
        df_.assert_equals(actual, df)
        
    def test_overlap_raises(self, session):
        '''
        When families overlap (before gene mapping), raise ValueError
        '''
        # Note: a config option for alternative handling could later be added:
        # ignore overlapping families (add neither) and warn
        # overlap within data frame
        with pytest.raises(ValueError) as ex:
            _interpret.gene_families(session, pd.DataFrame(
                [
                    ['fam1', 'gene1'],
                    ['fam1', 'gene2'],
                    ['fam2', 'gene2']
                ],
                columns=['family', 'gene']
            ))
        assert (
            dedent('''\
                gene_families contains overlap:
                family   gene
                 fam1  gene2
                 fam2  gene2'''
            ) in str(ex.value)
        )
        
    def test_overlap_gene_mapping(self, session):
        '''
        When genes overlap (after gene mapping), raise ValueError
        '''
        session.add_gene_mapping(pd.DataFrame(
            [
                ['geneA1', 'geneB1'],
                ['geneA1', 'geneB2'],
            ],
            columns=['source', 'destination']
        ))
        
        df = pd.DataFrame(
            [
                ['fam1', 'geneA1'],
                ['fam2', 'geneB2'],
            ],
            columns=['family', 'gene']
        )
        with pytest.raises(ValueError) as ex:
            _interpret.gene_families(session, df)
        assert (
            dedent('''\
                gene_families contains overlap after gene mapping:
                family    gene
                 fam1  geneB2
                 fam2  geneB2'''
            ) in str(ex.value)
        )
        
    def test_invalid_family_name_not_str(self, session):
        '''
        When family name is not a str, raise ValueError
        '''
        df = pd.DataFrame(
            [
                [None, 'gene1'],
                [np.nan, 'gene2'],
                [1, 'gene3'],
                ['ok', 'gene4']
            ],
            columns=['family', 'gene']
        )
        with pytest.raises(ValueError) as ex:
            _interpret.gene_families(session, df)
        expected = dedent('''\
            Gene family names must be strings. Got:
            family     gene
             None  'gene1'
              nan  'gene2'
                1  'gene3'
            '''.rstrip()
        )
        assert expected in str(ex.value)
        
    def test_invalid_family_name_str(self, session):
        '''
        When family name is empty or contains \0, raise ValueError
        '''
        df = pd.DataFrame(
            [
                ['', 'gene1'],
                ['h\0k', 'gene2'],
                ['ok', 'gene3']
            ],
            columns=['family', 'gene']
        )
        with pytest.raises(ValueError) as ex:
            _interpret.gene_families(session, df)
        expected = dedent('''\
            Gene family names must not be empty or contain a null character (\\0). Got:
            family     gene
                  ''  'gene1'
            'h\\x00k'  'gene2'
            '''.rstrip()
        )
        assert expected in str(ex.value)
            