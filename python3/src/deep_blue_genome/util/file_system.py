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
from deep_blue_genome.core.util import flatten

'''
Plumbum additions, basically
'''

def path_to_list(path):
    '''
    If dir, path.list(), else path
    '''
    return path.list() if path.isdir() else [path]

def flatten_paths(paths):
    '''
    Flatten 1 level of irregular list of paths
    
    Analog to flattening irregular lists where a directory is a list.
    
    Parameters
    ----------
    paths : iterable of plumbum.Path
    '''
    return flatten(list(map(path_to_list, paths)))