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
from itertools import chain
from collections import defaultdict
from pprint import pprint

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

    
    
    