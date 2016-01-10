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
Correlation functions such as Pearson R and functions to apply them
'''
 
import numpy as np
import pandas as pd

# XXX don't assert for no NaNs here, do that in the alg. Do add func to remove variance and other cleaning which belongs in some core.prepare for data prep funcs
# XXX docstrings

# XXX would be nice to have an interface to wrap around e.g. pearson_r to say 'these indices is the subset, restore index afterwards'. Share this func modified with coexpnetviz.
# XXX coexpnetviz should use this func. Fix coexpnetviz when done
def get_correlations(matrix, subset, correlation_method):
    mask = matrix.index.isin(subset)
    correlations = correlation_method(matrix.values, np.flatnonzero(mask))
    correlations = pd.DataFrame(correlations, index=matrix.index, columns=matrix.index[mask])
    return correlations

# XXX something to grab random subset of index, then of column, then a func to make corrs easily by (matrix, subset, subset2) where by default subset is everything of matrix. Input is DataFrame and df[compatible] because keeping it simple. Better yet might be (matrix, columns, rows=None) and require an actual index for columns,rows.
# XXX coexpnetviz should take random 800 as rows, random 800 from same set as columns. Unless coexpr doesn't compare to the same, which I think it doesn't, then should pick random first set and random second set that is disjoint with the first. Or do allow from the same but remove any self comparisons. That removal should be done by the caller though...
def get_correlations_sample(matrix, correlation_method, sample_size=800): #XXX doesn't return a dataframe, so can't compare it to get_correlations. Either return a DF (in which case you can indeed forward to get_correlations which is pretty nice) or ...
    '''
    Randomly select rows from matrix as subset and returns as if
    `get_correlations(matrix, subset, correlation_method)` was called.
    
    Parameters
    ----------
    matrix : numpy 2d-array of float
    correlation_method
    
    Returns
    -------
    np.array(dtype=float, shape=(len(data), len(subset)))
        Correlations. Position (i,j) contains correlation between row i and j'th
        subset row.
    '''
    data = matrix.values
    sample = np.random.choice(len(data), sample_size)
    sample = data[sample]
    sample = correlation_method(sample, np.arange(len(sample)))
    return sample
    
def pearson_r(data, subset):
    '''
    Pearson r-values between rows of `data` and rows of its subset ``data[subset]``.
    
    Parameters
    ----------
    data : 2d array of float
    subset : array of int
        Indices to subset of rows in `data` 
    
    Returns
    -------
    np.array(dtype=float, shape=(len(data), len(subset)))
        Correlations. Position (i,j) contains correlation between row i and j'th
        subset row.
    '''
    # This is a vectorised form of gsl_stats_correlation's algorithm.
    matrix = data
    mean = matrix[:,0].copy()
    delta = np.empty(matrix.shape[0])
    sum_sq = np.zeros(matrix.shape[0])  # sum of squares
    sum_cross = np.zeros((matrix.shape[0], len(subset)))
 
    for i in range(1,matrix.shape[1]):
        ratio = 1 / (i+1)
        delta = matrix[:,i] - mean
        sum_sq += (delta**2) * ratio;
        sum_cross += np.outer(delta, delta[subset]) * ratio;
        mean += delta / (i+1);
  
    sum_sq = np.sqrt(sum_sq)
    correlations = sum_cross / np.outer(sum_sq, sum_sq[subset]);
    return correlations

# def mutual_information(data, subset, metric):
#     '''
#     TODO
#     '''
#     # A more vectorised version of https://github.com/scikit-learn/scikit-learn/blob/c957249/sklearn/metrics/cluster/supervised.py#L507
#     if contingency is None:
#         labels_true, labels_pred = check_clusterings(labels_true, labels_pred)
#         contingency = contingency_matrix(labels_true, labels_pred)
#     contingency = np.array(contingency, dtype='float')
#     contingency_sum = np.sum(contingency)
#     pi = np.sum(contingency, axis=1)
#     pj = np.sum(contingency, axis=0)
#     outer = np.outer(pi, pj)
#     nnz = contingency != 0.0
#     # normalized contingency
#     contingency_nm = contingency[nnz]
#     log_contingency_nm = np.log(contingency_nm)
#     contingency_nm /= contingency_sum
#     # log(a / b) should be calculated as log(a) - log(b) for
#     # possible loss of precision
#     log_outer = -np.log(outer[nnz]) + log(pi.sum()) + log(pj.sum())
#     mi = (contingency_nm * (log_contingency_nm - log(contingency_sum))
#           + contingency_nm * log_outer)
#     return mi.sum()

def similarity_matrix(data, subset, metric):
    '''
    Get similarity between rows of `data` and rows of its subset ``data[subset]``.
    
    Parameters
    ----------
    data : 2d array of float
    subset : array of int
        Indices to subset of rows in `data`
    metric : function((x: array-like), (y: array-like)) -> float
        Similarity metric to use.
    
    Returns
    -------
    array(dtype=float, shape=(len(data), len(subset)))
        Similarity 2d array. Position (i,j) contains similarity between row i and j'th
        subset row.
    '''
    # Note: can be sped up (turn into metric specific linalg, or keep generic and use np ~enumerate). For vectorising mutual info further, see https://github.com/scikit-learn/scikit-learn/blob/c957249/sklearn/metrics/cluster/supervised.py#L507
    return np.array([np.apply_along_axis(metric, 1, data, data[item]) for item in subset]).T         
    # TODO test this by providing a pearson-r as metric func, then compare that to our pearson_r

