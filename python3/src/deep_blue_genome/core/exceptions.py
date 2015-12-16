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
Exceptions used by `core`
'''

class TaskFailedException(Exception):
    
    '''
    Raised when the current task should fail due to some (hopefully) exceptional
    event.
    
    To give an idea of the scope of these 'tasks', we give a few examples: a
    MORPH run; importing a gene expression matrix.
    '''
    
    def __init__(self, cause=None):
        super().__init__(self, "Task failed.\nCause: {}".format(cause))
        
class GeneNotFoundException(Exception):
    def __init__(self, gene_name, cause=None):
        super().__init__(self, "Could not find gene with name '{}'.\nCause: {}".format(gene_name, cause))