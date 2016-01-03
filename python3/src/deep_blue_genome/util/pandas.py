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
Helper functions for pandas collections
'''

import numpy as np

# TODO does df.values cost copy time?

# XXX have a count_nan or such added to DataFrame with this implementation
def pd_count_null(x):
    '''
    Parameters
    ----------
    x : pd.DataFrame or pd.Series
    
    Returns
    -------
    int
    '''
    return x.isnull().values.sum()
 
def df_count_null(df): # XXX rm
    return df.isnull().values.sum()

def pd_has_null(x): # XXX rm
    '''
    Parameters
    ----------
    x : pd.DataFrame or pd.Series
    
    Returns
    -------
    bool
    '''
    return x.isnull().values.any()

def df_has_null(df): # XXX rm
    return df.isnull().values.any()

def series_has_duplicates(series):
    return len(np.unique(series.values)) != len(series)