# Copyright (C) 2015, 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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
Plumbum additions
'''

def pb_with_stem(path, stem):
    '''
    Like `pb.Path.with_name`, but operate on the stem
    '''
    return path.with_name(stem + ''.join(path.suffixes))
 
def list_files(paths, filter_=None):
    '''
    List files recursively in paths
    
    Parameters
    ----------
    paths : iterable of plumbum.Path
        Paths to files and directories. If a directory, it is searched recursively for more files.
    filter_ : predicate(plumbum.Path) -> bool
        Only search in and return paths for which filter_ returns True. E.g.
        when your predicate returns False for hidden files, we will not search
        in '.hg', nor return '.hg/nothiddenfile'.
    '''
    for p in paths:
        if filter_(p):
            if p.isdir():
                for p in list_files(p.list(), filter_): #XXX switch to iterdir and set plumbum>=1.6.1
                    yield p
            else:
                yield p
            
            