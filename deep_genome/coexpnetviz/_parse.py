# Copyright (C) 2015, 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Genome.
# 
# Deep Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Genome.  If not, see <http://www.gnu.org/licenses/>.

from deep_genome.core import clean, parse
from chicken_turtle_util import data_frame as df_
import pandas as pd

def gene_families(reader, name_index=0):
    '''
    Import gene families file into database
    
    Parameters
    ----------
    reader : file object
        Text reader whose content is a gene families file, i.e. a clustering
        with families as clusters
    name_index : int
        The index of the gene family name field on each row. See `deep_genome.core.parsers.Parser.parse_clustering`
        
    Returns
    -------
    pd.DataFrame({'family' => [str], 'gene' => [str]})
    '''
    clustering = parse.clustering(reader, name_index)
    clustering = pd.DataFrame(list(clustering.items()), columns=('family', 'gene'))
    clustering['gene'] = clustering['gene'].apply(list)
    clustering = df_.split_array_like(clustering, 'gene')
    return clustering
