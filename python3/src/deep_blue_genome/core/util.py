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

import pandas
import numpy as np
from itertools import chain
from collections import defaultdict
from pprint import pprint
import colorsys

def is_sorted(l):
    return all(l[i] <= l[i+1] for i in range(len(l)-1))

def fill_na_with_none(df):
    '''
    Fill all NaN in DataFrame with None.
    
    These None values will not be treated as 'missing' by DataFrame, as the dtypes will be set to 'object'
    '''
    df.where(pandas.notnull(df), None, inplace=True)
    
def flatten(lists):
    '''
    Flatten shallow list
    
    Parameters
    ----------
    list-like of list-like
        Shallow list
    '''
    return list(chain(*lists))

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
        df = pandas.DataFrame(expanded, columns=df.columns)
    return df

def series_swap_with_index(series):
    '''
    Swap index with values of series
    
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

def get_distinct_colours(n):
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
    
if __name__ == '__main__':
    df = pandas.DataFrame([[1,[1,2],[1]],[1,[1,2],[3,4,5]],[2,[1],[1,2]]], columns='check a b'.split())
    print(df)
    print(df_expand_iterable_values(df, ['a', 'b']))
    