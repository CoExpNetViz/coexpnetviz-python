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

from deep_blue_genome.test.util import CLITester
import pytest

cli = CLITester('coexpnetviz'.split())

# @pytest.mark.current
class TestCoExpNetViz(object):
    
    '''
    CoExpNetViz tests
    '''
        
    #TODO 1 species custom
    # TODO check output automatically
    def test_custom_fam_2_species(self, tmpdir):
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits',
                expression_matrices=['caros_expression_matrix', 'itag_expression_matrix'],
                gene_families_file='gene_families'
            ),
            'custom_fams'
        )
        cli.run(**files)
        
    def test_missing_bait(self, tmpdir):
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits_missing',
                expression_matrices=['caros_expression_matrix', 'itag_expression_matrix'],
                gene_families_file='gene_families'
            ),
            'custom_fams'
        )
        cli.run(**files)
        
    def test_plaza_1_species(self, tmpdir):
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits_one_species',
                expression_matrices=['arabidopsis_expression_matrix'],
                gene_families_file='merged_plaza.gene_fams.txt'
            ),
            'plaza_fams'
        )
        cli.run(**files)
        
    def test_plaza_1_species_no_genefam(self, tmpdir):
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits_one_species',
                expression_matrices='arabidopsis_expression_matrix'
            ),
            'plaza_fams'
        )
        cli.run(**files)
        
    def test_plaza_2_species(self, tmpdir):
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits_two_species',
                expression_matrices=['arabidopsis_expression_matrix', 'apple_expression_matrix'],
                gene_families_file='merged_plaza.gene_fams.txt',
            ),
            'plaza_fams'
        )
        cli.run(
            #similarity_metric=SimilarityMetric.mutual_information,
            **files
        )
        
    def test_plaza_2_species_percentiles(self, tmpdir):
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits_two_species',
                expression_matrices=['arabidopsis_expression_matrix', 'apple_expression_matrix'],
                gene_families_file='merged_plaza.gene_fams.txt',
            ),
            'plaza_fams'
        )
        cli.run(
            lower_percentile_rank=1,
            upper_percentile_rank=99,
            **files
        )
        
        