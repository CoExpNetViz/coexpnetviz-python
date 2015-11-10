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

from Bio import Entrez
from deep_blue_genome.exceptions import GeneNotFoundException

# Gene lookup
#==============

# TODO provide a default data prep config that fetches the necessary files for DBG to function.
# TODO update the gene_info data file daily (or more slowly)

# We also need to be careful with failures to get a result (network hickups -> retry a couple of times, then give up).
# Latency might kill performance, but let's wait and see

class Genes(object):
    
    '''
    Represents a collection of genes in which can be searched using basic properties (no sequence search)
    '''
        
    def find_by_name(self, name):
        '''
        Find gene by name
        
        Returns
        -------
        Gene
            Found gene
            
        Raises
        ------
        KeyNotFoundError
        '''
        search_result = Entrez.read(Entrez.esearch("gene", term="{0}[Gene Name]".format(name)))
        ids = search_result['IdList']
        if len(ids) > 1:
            raise NotImplemented  # TODO Could throw something, could take the first, could use something somewhere as tiebreaker. Could make the behaviour configurable
        elif not ids:
            raise GeneNotFoundException
        else:
            gene = list(Entrez.parse(Entrez.efetch("gene", id=ids[0], retmode="xml")))[0]
            return gene  # TODO Gene class or something probably desired, initially with just the name and species or some ids

