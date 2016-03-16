# Copyright (C) 2015, 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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
import plumbum as pb

cli = CLITester('morph'.split())
        
class TestMORPH(object):
    
    '''
    MORPH tests
    ''' 
           
    # TODO Tests:
    # - Are the correct clusterings an matrices selected? Think of min amount of baits that need to be present. Simply don't mention those that did not qualify. If none qualify, do mention that, duh.
    # - Sanity check on output: AUSR in valid range? Scores in valid range (not NaN)?
    # - Compare to an old MORPH run: run with your stuff, take selected list and feed it as a job to old MORPH, then you can compare outputs.
    @pytest.mark.current 
    def test_tmp(self, tmpdir):
        '''
        No matrices or clusterings are matched
        '''
        tmp_dir = pb.local.path(tmpdir)
        output_dir = tmp_dir / 'output'
        output_dir.mkdir()
        cli.tmpdir = tmp_dir # XXX can't concurrent test run with the same CLITester when written like this. Either fixture with scope or fix.
        cli.run(
#             '/mnt/data/doc/work/prod_data/ARABIDOBSIS/gois',
#             '/mnt/data/doc/work/prod_data/ARABIDOBSIS/pathways',
            '/mnt/data/doc/work/prod_data/rice/gois',
            output_dir=output_dir,
        )
        
    # TODO check only pub data used for testing 
         
#     def test_no_match(self, tmpdir):
#         '''
#         No matrices or clusterings are matched
#         '''
#         self.run(tmpdir, prefix=data_dir / 'plaza_fams',
#             baits_file='baits_two_species',
#         )

