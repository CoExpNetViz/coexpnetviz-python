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

import numpy as np
import colorsys
from numpy.linalg import norm


def remove_duplicates(items):
    '''
    Removes duplicates in list, order is *not* preserved
    
    Returns uniqued list, does *not* modify original list 
    '''
    return list(set(items))


     
# See https://en.wikipedia.org/wiki/YUV#HDTV_with_BT.601
_yuv_to_rgb = np.matrix([
    [1, 0, 1.28033],
    [1, -0.21482, -0.38059],
    [1, 2.12798, 0]
]).T

def yuv_to_rgb(yuv):
    '''
    HDTV-Y'UV point to RGB color point
    
    Note that not all YUV values between [0,0,0] and [1,1,1] map to valid rgb
    values (i.e. some fall outside the [0,1] range) (see p30 http://www.compression.ru/download/articles/color_space/ch03.pdf)
    
    Parameters
    ----------
    yuv : array-like
        An (n,3) shaped array. YUV point per row.
    
    Returns
    -------
    array-like
        RGB point per row.
    '''
    return yuv * _yuv_to_rgb

def get_distinct_colours(n):
    '''
    Get `n` most distinguishably colours as perceived by human vision.
    
    No returned colour is entirely black, nor entirely white.
    
    Based on: http://stackoverflow.com/a/30881059/1031434
    
    Returns
    -------
    iterable of (r,g,b)
        n RGB colours
    ''' 
    points = spread_points_in_hypercube(n+2, 3)
    lightest = norm(points, axis=1).argmax()
    darkest = norm(points - np.array([1,1,1]), axis=1).argmax()
    return np.delete(points, np.array([lightest,darkest]), axis=0)
    # TODO use CIEDE2000 or the simpler CMC l:c.
    # https://en.wikipedia.org/wiki/Color_difference
    # The trick lies in
    # intersecting a regular space to the part of the color space that maps back
    # to valid rgb values and hoping you are left with enough points.
#     # TODO to avoid black or white, scale down the Y component to [0.1, 0.9]
#     return yuv_to_rgb(points)

def get_distinct_colours_hsv(n):
    '''
    Gets n distinct colours based on hue (HSV)
    
    It does not take into account human perception of colour differences as YUV
    might.
    
    No returned colour is black, nor white.
    
    Source: http://stackoverflow.com/a/876872/1031434
    
    Returns
    -------
    iterable of (r,g,b)
        n RGB colours
    '''
    hsv_colours = [(x/n, 0.5, 0.5) for x in range(n)]
    return map(lambda x: colorsys.hsv_to_rgb(*x), hsv_colours)
    