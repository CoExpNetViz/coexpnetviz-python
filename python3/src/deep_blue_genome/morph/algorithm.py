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
    
def morph(context, baits, top_k):
    '''
    TODO
    
    TODO
    
    Parameters
    ----------
    baits : see `read_baits_file`'s return value
        genes to which non-bait genes are compared
    top_k : int
        K best candidate genes to output in ranking
        
    Returns
    -------
    TODO
        TODO
    '''
    # fetch list of relevant clusterings and expression matricess from DB
    # Main alg stuff
    
    