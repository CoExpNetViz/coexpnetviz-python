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

import pandas as pd
import numpy as np
import logging

_logger = logging.getLogger('deep_blue_genome.morph')
    
def morph(context, bait_groups, top_k):
    '''
    TODO
    
    TODO
    
    Parameters
    ----------
    bait_groups : pd.DataFrame(columns=(group_id : int, gene : Gene))
        list of gene collections to which non-bait genes are compared
    top_k : int
        K best candidate genes to output in ranking
        
    Returns
    -------
    TODO
        TODO
    '''
    db = context.database
    
    # fetch list of relevant clusterings and expression matricess from DB
    _logger.info('Using expression matrices and clusterings which contain at least 5 baits (per bait set)')
    result = db.get_by_genes(bait_groups, min_genes_present=5, expression_matrices=True, clusterings=True)
    print(result.expression_matrices)
    print(result.clusterings)
    
    # Main alg stuff
#     read_expression_matrix
#     read_clustering
    # AUSR...
    
    