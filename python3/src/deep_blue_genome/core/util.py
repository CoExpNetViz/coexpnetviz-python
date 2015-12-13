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

import pandas as pd
import math
import numpy as np
from itertools import chain
from collections import defaultdict
from pprint import pprint
import colorsys
from sklearn.utils.extmath import cartesian
from numpy.linalg import norm
from functools import reduce

def is_sorted(l):
    return all(l[i] <= l[i+1] for i in range(len(l)-1))

def fill_na_with_none(df):
    '''
    Fill all NaN in DataFrame with None.
    
    These None values will not be treated as 'missing' by DataFrame, as the dtypes will be set to 'object'
    '''
    df.where(pd.notnull(df), None, inplace=True)
    
def flatten(list_):
    '''
    Flatten one level of a regularly nested list
    
    Parameters
    ----------
    list_ : list-like of list-like
    '''
    return list(chain(*list_))

def flatten_deep(list_):
    '''
    Flatten list deeply
    
    Irregularly nested lists are flattened as well:
    
    >>> flatten_deep([5, [1,2], 6, [7,[8]]])
    [5,1,2,6,7,8]
    
    Parameters
    ----------
    list_ : list or any
        If list, it is flattened, otherwise it's returned as is
    '''
    if isinstance(list_, list):
        return list(map(flatten_deep, list_))
    else:
        return list_

def remove_duplicates(items):
    '''
    Removes duplicates in list, order is *not* preserved
    
    Returns uniqued list, does *not* modify original list 
    '''
    return list(set(items))

def invert_multidict(dict_):
    '''
    Invert multi-dict (=multimap)
    
    Parameters
    ----------
    dict_ : {any -> iterable}
        Multi-dict to invert
        
    Returns
    -------
    {any -> {any}}
        Inverted dict
    '''
    result = defaultdict(lambda: set())
    for k, vals in dict_.items():
        for val in vals:
            result[val].add(k)
    return dict(result)

def invert_dict(dict_):
    '''
    Invert dict
    
    Parameters
    ----------
    dict_ : {any -> hashable}
        Dict to invert
        
    Returns
    -------
    {any -> {any}}
        Inverted dict
    '''
    result = defaultdict(lambda: set())
    for k, val in dict_.items():
        result[val].add(k)
    return dict(result)

def print_abbreviated_dict(dict_, count=10):
    pprint(dict(list(dict_.items())[:count]))
    
def df_expand_iterable_values(df, columns):
    '''
    Expand repeatably iterable values in given columns along row axis.
    
    Column names are maintained, the index is dropped.
    
    >>> pandas.DataFrame([[1,[1,2],[1]],[1,[1,2],[3,4,5]],[2,[1],[1,2]]], columns='check a b'.split())
       check       a          b
    0      1  [1, 2]        [1]
    1      1  [1, 2]  [3, 4, 5]
    2      2     [1]     [1, 2]
    >>> df_expand_iterable_values(df, ['a', 'b'])
      check  a  b
    0     1  1  1
    1     1  2  1
    2     1  1  3
    3     1  1  4
    4     1  1  5
    5     1  2  3
    6     1  2  4
    7     1  2  5
    8     2  1  1
    9     2  1  2
    
    Parameters
    ----------
    df : pandas.DataFrame
    columns : iterable of str, or str
        Columns (or column) to expand. Their values should be repeatably iterable.
        
    Returns
    -------
    pandas.DataFrame
        Data frame with values in `columns` split to rows
    '''
    #TODO could add option to keep_index by using reset_index and eventually set_index. index names trickery: MultiIndex.names, Index.name. Both can be None. If Index.name can be None in which case it translates to 'index' or if that already exists, it translates to 'level_0'. If MultiIndex.names is None, it translates to level_0,... level_N
    if isinstance(columns, str):
        columns = [columns]
        
    for column in columns:
        expanded = np.repeat(df.values, df[column].apply(len).values, axis=0)
        expanded[:, df.columns.get_loc(column)] = np.concatenate(df[column].tolist())
        df = pd.DataFrame(expanded, columns=df.columns)
    return df

def series_invert(series):
    '''
    Swap index with values of series
    
    When planning to join 2 series, use `pandas.Series.map` instead of swapping.
    
    Parameters
    ----------
    series
        Series to swap on, must have a name
    
    Returns
    -------
    pandas.Series
        Series after swap 
    '''
    df = series.reset_index() #TODO alt is to to_frame and then use som dataframe methods
    df.set_index(series.name, inplace=True)
    return df[df.columns[0]]
    
class keydefaultdict(defaultdict):
    
    '''
    Like defaultdict, but its default value factory takes a key argument
    
    Source: http://stackoverflow.com/a/2912455/1031434
    '''
    
    def __missing__(self, key):
        if self.default_factory is None:
            raise KeyError(key)
        else:
            ret = self[key] = self.default_factory(key)
            return ret

def spread_points_in_hypercube(point_count, dim_count):
    '''
    Approximate a spreading of `n` points in a unit hypercube of given dimension.
    
    Euclidean distance is used.
    
    Note that the exact solution to this problem is known for only a few `n`.
    
    Related: http://stackoverflow.com/a/2723764/1031434
    
    Parameters
    ----------
    point_count : int-like
        Number of points to pick
    dim_count : int-like
        Number of dimensions of the hypercube
        
    Returns
    -------
    array-like
        Points spread approximately optimally in the hypercube. I.e. each point
        component will lie in the interval [0,1].
    '''
    # Current implementation simply puts points in a grid
    side_count = math.ceil(point_count ** (1/dim_count)) # number of points per side
    points = np.linspace(0, 1, side_count)
    points = cartesian([points]*dim_count)
    return np.random.permutation(points)[:point_count]
     
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
    
##############
# util.functools
def compose(*functions):
    '''
    Compose functions
    
    E.g. compose(f1, f2) returns f1 o f2, i.e. lambda x: f1(f2(x))
    
    Parameters
    ----------
    functions : list-like of (function : func(x) -> y)
    '''
    apply = lambda x, y: y(x) 
    return lambda x: reduce(apply, functions, x)

def dict_subset(dict_, keys, fragile=True):
    '''
    Get subset of dict as dict
    
    When keys are missing, KeyError is raised.
    
    Parameters
    ----------
    dict_ : dict
        Dict to take subset from
    keys : iterable of str
        Keys to include in subset
    fragile : bool
        If True, raise on missing key, else omits missing keys from subset.
    ''' 
    if fragile:
        return {k : dict_[k] for k in keys}
    else:
        return {k : dict_[k] for k in keys if k in dict_}

if __name__ == '__main__':
    df = pd.DataFrame([[1,[1,2],[1]],[1,[1,2],[3,4,5]],[2,[1],[1,2]]], columns='check a b'.split())
    print(df)
    print(df_expand_iterable_values(df, ['a', 'b']))
    