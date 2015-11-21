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

class ExpressionMatrix(object):
    
    def __init__(self, data, name=None):
        self._name = name
        self._data = data
        
    @property
    def name(self):
        return self._name
    
    @property
    def data(self):
        '''
        Returns
        -------
        pandas.DataFrame
            Expression data with gene names as row labels and condition (in which it was tested) names as column names
        '''
        return self._data
    
    def pearson_r(self, subset): #TODO name and params should be...?
        '''
        Pearson r-values between rows of `self.data` and rows of its subset ``self.data.loc(subset)``.
        
        Parameters
        ----------
        subset : list-like of labels
        
        Returns
        -------
        pandas.DataFrame
            Correlations. Position (i,j) contains correlation between row i and j'th
            subset row. Returned frame has ``self.data.index`` as index and `subset` as
            column names.
        '''
        # TODO correlation between e.g. [1,1] and [2,2] is NaN with pearson, which kind of makes sense because... how could you possibly tell? Is it 1, -1, something in between?
     
        # This is a vectorised form of gsl_stats_correlation's algorithm.
        index = self.data.index
        bait_indices = np.array([index.get_loc(bait) for bait in subset])
        matrix = self.data.astype(float).values
        mean = matrix[:,0].copy()
        delta = np.empty(matrix.shape[0])
        sum_sq = np.zeros(matrix.shape[0])  # sum of squares
        sum_cross = np.zeros((matrix.shape[0], len(subset)))
     
        for i in range(1,matrix.shape[1]):
            ratio = 1 / (i+1)
            delta = matrix[:,i] - mean
            sum_sq += (delta**2) * ratio;
            sum_cross += np.outer(delta, delta[bait_indices]) * ratio;
            mean += delta / (i+1);
      
        sum_sq = np.sqrt(sum_sq)
        correlations = sum_cross / np.outer(sum_sq, sum_sq[bait_indices]);
        correlations = pandas.DataFrame(correlations, columns=subset, index=index)
        return correlations
        

