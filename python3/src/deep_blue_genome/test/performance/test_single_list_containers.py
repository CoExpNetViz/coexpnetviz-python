
# TODO may be set does not belong in this list. Also note that Series does not have set operations, but pd.Index does
'''
Performance of containers that can act like a single list or set

Results:

- Construction speed (fastest to slower): list (1-3), np.array, set (5-11), series(128), data frame (191)

- list and set are more lightweight than Series and DataFrame (as you would
  expect, the latter have many more features). Sets are only slightly faster to
  construct than Series

- np.array is 20x slower to construct than a list, but has faster operations

- Series and DataFrame construction is 2x slower than np.array.

- DataFrame's construction is only slightly slower than a Series'. It's negligible.

- Converting a Series to DataFrame incurs little more than copy cost.

- Constructing list from list or set from set is faster than constructing
  from another collection type. It comes close to copy() time if the input is
  passed in as an iterable.
  
- Constructing list from np.array

- x.tolist() 2x faster than list(x) for np.array. For Series it makes no difference.

- series.values takes negligible time

- np.array.tolist() is 7x slower than constructing list from list
  
- x.copy() is eager, it copies immediately

- Copy performance: np.array (1), list (14), Series (34), set (56), DataFrame (73)

- deepcopy is slow as it needs to check for lots of edge cases, avoid it

Conclusion:

- Don't use np.array, Series or DataFrame, unless you will need the features
  they offer somewhere, at least once. The specialised methods they offer are
  faster than any version you'd write (and if not you should submit a patch to
  numpy/pandas).
  
- If you do need something, construct it as such from the start.
  
- When possible, use a Series/DataFrame's values. `np.array` is faster. When
  pandas has a specific method for it that np does not have, use that.
  
- When converting generic inputs, perform an isinstance check to avoid an unnecessary copy.
  If you need a copy regardless, constructor(iter(x)) is 2x faster than
  constructor(x) for constructor=set|list.
'''

# TODO some of the above could warrant patches to pandas

import pytest
import pandas as pd
import numpy as np

input_ = list(range(3000))
single_input_list = list(input_)
single_input_set = set(input_)
series = pd.Series(single_input_list)
data_frame = pd.DataFrame(single_input_list)
np_array = np.array(single_input_list)
del input_

def list_mutate(x):
    x[0] = 1337

def set_mutate(x):
    x.pop()
    x.add(1337)

def series_mutate(x):
    x.iloc[0] = 1337

def data_frame_mutate(x):
    x.iloc[0] = 1337
    
def np_mutate(x):
    x[0] = 1337  

collections = [single_input_list, single_input_set, series, data_frame, np_array]
collection_ids = 'list set Series DataFrame np.array'.split()
mutators = [list_mutate, set_mutate, series_mutate, data_frame_mutate, np_mutate]
    
@pytest.mark.skip_unless_current('benchmark')
# @pytest.mark.current
class TestConstruction(object):
    
    '''Performance of constructing an instance'''
    
    @pytest.mark.parametrize('input_', [single_input_list, single_input_set, series, np_array], ids='list set series np.array'.split())
    @pytest.mark.parametrize('iter_', [False, True], ids='iter no_iter'.split())
    @pytest.mark.parametrize('constructor', [list, set])
    def test_construct_std(self, constructor, input_, iter_, benchmark):
        self._test_construct(constructor, input_, iter_, benchmark)
        
    @pytest.mark.parametrize('constructor', [pd.Series, pd.DataFrame])
    def test_construct_pandas(self, constructor, benchmark):
        self._test_construct(constructor, single_input_list, False, benchmark)
        
    def test_construct_np_array(self, benchmark):
        self._test_construct(np.array, single_input_list, False, benchmark)
        
    def _test_construct(self, constructor, input_, iter_, benchmark):
        if iter_:
            actual = benchmark(lambda x: constructor(iter(x)), input_)
        else:
            actual = benchmark(lambda x: constructor(x), input_)
        if isinstance(actual, pd.DataFrame):
            actual = actual[0]
        assert single_input_set == set(actual)
         
    def test_series_from_data_frame(self, benchmark):
        benchmark(lambda x: x[0].copy(), data_frame)
         
    def test_data_frame_from_series(self, benchmark):
        benchmark(lambda x: x.to_frame(), series)
        
    def test_series_to_list(self, benchmark):
        benchmark(lambda x: x.tolist(), series)
        
    def test_series_to_list_via_np(self, benchmark):
        benchmark(lambda x: x.values.tolist(), series)
        
    def test_np_array_to_list(self, benchmark):
        benchmark(lambda x: x.tolist(), np_array)

@pytest.mark.skip_unless_current('benchmark')
class TestCopy(object):
    
    '''Performance of x.copy()'''
    
#     @pytest.mark.current
    @pytest.mark.parametrize('collection', collections, ids=collection_ids)
    def test_shallow_copy(self, collection, benchmark): # aka list_to_list
        benchmark(lambda x: x.copy(), collection)
        
    @pytest.mark.parametrize('collection,mutator', list(zip(collections, mutators)), ids=collection_ids)
    def test_shallow_copy_mutate(self, collection, mutator, benchmark): # aka list_to_list
        benchmark(lambda x: mutator(x.copy()), collection)
      
    @pytest.mark.parametrize('collection,mutator', list(zip(collections, mutators)), ids=collection_ids)
    def test_mutate(self, collection, mutator, benchmark): # aka list_to_list
        benchmark(mutator, collection.copy())
         
         
# TODO TestSetOperations, are Series set ops faster?