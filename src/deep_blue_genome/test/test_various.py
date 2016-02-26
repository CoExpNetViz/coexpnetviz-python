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

from deep_blue_genome.core.util import get_distinct_colours,\
    spread_points_in_hypercube
import pandas as pd
import matplotlib.pyplot as plt

# TODO split files: per interface, various for the rest
# TODO Base test class for exercising the CLI through the main func with args specified in python

def manual_test_distinct_colors():
    n=50
    colors = list(get_distinct_colours(n))
    print(colors)
    pd.Series([1]*n).plot.pie(colors=colors)
    plt.show()
    
def test_spread_points():
    for n, dims in [(4,2), (9,3), (8,2), (5,3)]:
        result = list(spread_points_in_hypercube(n, dims))
        print(result)
        assert len(result) == n
        assert len(result[0]) == dims

    