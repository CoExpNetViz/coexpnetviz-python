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
Test coexpnetviz._various.get_distinct_colours()
'''

import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import numpy as np
import pytest

from coexpnetviz._various import distinct_colours

@pytest.fixture(autouse=True)
def use_temp_dir_cwd(temp_dir_cwd):
    pass

class TestDistinctColours:

    def test_regression(self):
        '''
        Happy days regression test
        '''
        colours = distinct_colours(9)
        expected = np.array([
            [0.94022, 0.831869, 0.2684288],
            [0.94022, 0.762981, 0.6228096],
            [0.54022, 0.294093, 0.5771904],
            [0.82066, 0.151167, 0.5771904],
            [0.25978, 0.368131, 0.9315712],
            [0.82066, 0.220055, 0.2228096],
            [0.25978, 0.505907, 0.2228096],
            [0.54022, 0.225205, 0.9315712],
            [0.37934, 0.979945, 0.9771904],
        ])
        assert np.allclose(colours, expected)

    @pytest.mark.manual
    def test_manual(self):
        '''
        Happy days scenario showing grid of colours
        '''
        # Get distinct colours
        grid_size = 5
        colours = distinct_colours(grid_size**2)

        # Show colors in a grid for manual inspection
        _, ax = plt.subplots()
        grid = np.mgrid[0.2:0.8:grid_size*1j, 0.2:0.8:grid_size*1j].reshape(2, -1).T
        rect_size = grid[1,1] - grid[0,1]
        rect_size = np.array([rect_size, rect_size])
        for i, colour in enumerate(colours):
            rect = mpatches.Rectangle(grid[i] - rect_size/2, *rect_size, ec="none", color=colour)
            ax.add_patch(rect)
        plt.subplots_adjust(left=0, right=1, bottom=0, top=1)
        plt.axis('equal')
        plt.axis('off')
        plt.show()
