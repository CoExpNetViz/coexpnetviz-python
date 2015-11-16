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
Configuration, performance tuning parameters mostly
'''

class Config(object):
    
    @property
    def disk_read_speed(self):
        '''HDD read speed in bytes/s''' 
        return 128 * 2**20
    
    @property
    def max_dataframe_size(self):
        '''Max size returned data frames should have, to avoid memory usage explosions'''
        return self.disk_read_speed
        #return 16 * 2**20
    
config = Config()
del Config