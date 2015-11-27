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
import numpy as np
from scipy.constants.constants import metric_ton

# TODO correlation between e.g. [1,1] and [2,2] is NaN with pearson, which kind of makes sense because... how could you possibly tell? Is it 1, -1, something in between?
    
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
    array(dtype=float, shape=(len(data), len(subset)))
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

