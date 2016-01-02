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

'''
Performance comparison of collection of sets: [np.array] vs [set] vs [Series] vs DataFrame

Also known as a set family or in some contexts a clustering. 

Results:

- Set operation performance (union and intersection, fastest to slowest): set, np.array, Index, DataFrame. DataFrame is vastly slower.

- DataFrame union is still vastly slower when omitting drop_duplicates.

- Construction performance: set (5), np.array (18), DataFrame (40), Index (131)

Conclusion:

- When in need of set operations, set is preferred. np.array is roughly twice as
  slow as set, so it may not be worth converting it to sets if you already have
  an np.array. However, DataFrame in relational format is vastly slower, it's better to
  convert it to sets (and back again).
'''

import pytest
import pandas as pd
import numpy as np
from deep_blue_genome.test.unittests.util.test_merge_overlapping_named_sets import create_input
from deep_blue_genome.core.util import df_expand_iterable_values
import itertools

def normalised_set_family(sets):
    return sorted(sorted(set_) for set_ in sets if list(set_))

inputs = create_input(0.1, 'uniform', 100, 1000)[0]
input_sets = normalised_set_family(inputs)

def construct_np_arrays(lists):
    return [np.unique(np.array(list_)) for list_ in lists]

def construct_sets(lists):
    return [set(list_) for list_ in lists]

def construct_index(lists):
    def f(list_):
        index = pd.Index(list_)
        index = index.drop_duplicates()  # no inplace
        return index
    return [f(list_) for list_ in lists]

def construct_data_frame(lists): # why data frame? Because no drop_duplicates for Series that takes into account the index (you would need to to_frame)
    assert isinstance(lists, list)
    df = pd.DataFrame([[list_] for list_ in lists], columns=['item'])
    df.index.name = 'set_id'
    df.reset_index(inplace=True)
    df = df_expand_iterable_values(df, ['item'])
    df.drop_duplicates(inplace=True)
    return df

def union_np_arrays(x):
    return [np.union1d(x[0], x[1])] + x[2:]

def union_sets(x):
    return [x[0] | x[1]] + x[2:]

def union_index(x):
    return [x[0] | x[1]] + x[2:]

def union_data_frame(df): # why data frame? Because no drop_duplicates for Series that takes into account the index (you would need to to_frame)
    df.set_id[df.set_id == 1] = 0
    df.drop_duplicates(inplace=True) # would be nice if we could hint it to only look in set_id 0
    return df

def intersect_np_arrays(x):
    return [np.intersect1d(x[0], x[1])] + x[2:]

def intersect_sets(x):
    return [x[0] & x[1]] + x[2:]

def intersect_index(x):
    return [x[0] & x[1]] + x[2:]

def intersect_data_frame(df): # why data frame? Because no drop_duplicates for Series that takes into account the index (you would need to to_frame)
    mask = df.set_id.isin([0,1])
    intersection = df[mask].groupby('item').apply(lambda x: len(x) == 2)
    intersection = pd.DataFrame(intersection.index[intersection], columns=['item'])
    intersection['set_id'] = 0
    df = df[~mask].append(intersection)
    return df

def intersect_data_frame_convert(df):
    mask = df.set_id.isin([0,1])
    sets = df[mask].groupby('set_id')['item'].apply(set).values.tolist()
    df = df[~mask]
    if len(sets) == 2: 
        intersection = pd.DataFrame(list(sets[0] & sets[1]), columns=['item']) #TODO sets[1] does not always exist
        intersection['set_id'] = 0
        df = df.append(intersection)
    return df

# XXX: set_index -> df.index.intersect ... 
# XXX via df.values?
     
# XXX np.array like df.values might be faster, but if so, we should instead patch DataFrame with a faster groupby implementation etc  http://stackoverflow.com/questions/4651683/numpy-grouping-using-itertools-groupby-performance
constructors = [construct_np_arrays, construct_sets, construct_index, construct_data_frame]
constructor_ids = '[np.array] [set] [Index] DataFrame'.split()
unioners = [union_np_arrays, union_sets, union_index, union_data_frame]

constructors_ = constructors + [construct_data_frame]
constructor_ids_ = constructor_ids + ['DataFrame_convert']
intersecters = [intersect_np_arrays, intersect_sets, intersect_index, intersect_data_frame, intersect_data_frame_convert]
    
@pytest.mark.skip_unless_current('benchmark')
# @pytest.mark.current
class TestConstruction(object):
    
    '''
    Performance of constructing an instance from a [list]
    
    Duplicate elements should be removed, duplicate sets should not be
    
    Results:
    
    - (iter(item) for item in items) is slightly faster (e.g. 3.26 vs 3.62)
    '''
    
    @pytest.mark.parametrize('constructor', constructors, ids=constructor_ids)
    def test_construct(self, constructor, benchmark):
        actual = benchmark(constructor, inputs)
        if isinstance(actual, pd.DataFrame):
            actual = actual.groupby('set_id')['item'].apply(list).values.tolist()
        assert input_sets == normalised_set_family(actual)
   
def create_2_overlapping_sets(left_diff, right_diff, size):
    intersect = 1 - left_diff - right_diff
    indices = (np.array([0, left_diff, intersect, right_diff]) * size).cumsum()
    indices = indices.astype(int)
    left = list(range(indices[0], indices[2]))
    right = list(range(indices[1], indices[3]))
    return [left, right]

create_2_overlapping_sets_args =  [(x,y) for x,y in itertools.product(*([[0, .1, .5, .9, 1]]*2)) if x+y <= 1]
overlapping_sets = [create_2_overlapping_sets(x, y, 1000) for x, y in create_2_overlapping_sets_args]

# XXX add sets in between and around the sets to union/intersect
@pytest.mark.skip_unless_current('benchmark')
# @pytest.mark.current
class TestSetOperations(object):
    
    '''Performance of set union and intersection'''
    
    @pytest.mark.parametrize('inputs', overlapping_sets, ids=[str(x) for x in create_2_overlapping_sets_args])
    @pytest.mark.parametrize('constructor,unioner', zip(constructors, unioners), ids=constructor_ids)
    def test_union(self, inputs, constructor, unioner, benchmark):
        set_family = constructor(inputs)
        actual = benchmark.pedantic(unioner, setup=lambda: ([set_family],{}))
        if isinstance(actual, pd.DataFrame):
            actual = actual.groupby('set_id')['item'].apply(list).values.tolist()
        assert normalised_set_family([set(inputs[0]) | set(inputs[1])]) == normalised_set_family(actual)
    
    
    @pytest.mark.parametrize('inputs', overlapping_sets, ids=[str(x) for x in create_2_overlapping_sets_args])
    @pytest.mark.parametrize('constructor,intersecter', zip(constructors_, intersecters), ids=constructor_ids_)
    def test_intersect(self, inputs, constructor, intersecter, benchmark):
        set_family = constructor(inputs)
        actual = benchmark.pedantic(intersecter, setup=lambda: ([set_family.copy()],{}))
        if isinstance(actual, pd.DataFrame):
            actual = actual.groupby('set_id')['item'].apply(list).values.tolist()
        assert normalised_set_family([set(inputs[0]) & set(inputs[1])]) == normalised_set_family(actual)
        