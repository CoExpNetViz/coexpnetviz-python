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

import pytest
from deep_blue_genome.main import main
import os
from deep_blue_genome.test.unittests.util.test_merge_overlapping_named_sets import group_results

# TODO all our tests are manual, can't we automate? (md5sums are a start, allows you to autotest without keeping the actual files in repo; still need input file though...)

def manual_test_cli():
    # Note: in some cases you may want: http://click.pocoo.org/6/api/#click.testing.CliRunner
#     main(['wrong'])
#     main(['--help'])
    main(['morph', '--help'])
    main(['prepare', '--help'])
#     main('prepare'.split())

#     manual_test_cli()    
#     manual_test_distinct_colors()

if __name__ == '__main__':
    args = (
        '-m current '
#         '-n auto --benchmark-disable'  # parallel testing (can't and shouldn't benchmark in parallel, so --benchmark-disable)
#         '--maxfail=1 '
#         '--capture=no '
    )
    pytest.main(args)
#     group_results()

#     manual_test_cli()    
#     manual_test_distinct_colors()
    
    
    
    