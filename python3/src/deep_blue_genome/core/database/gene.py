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

from deep_blue_genome.core.database.dbentity import DBEntity
from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import relationship
    
class GeneName(DBEntity):
    
    id =  Column(Integer, primary_key=True)
    name = Column(String(250), unique=True, nullable=False)
    gene_id =  Column(Integer, ForeignKey('gene.id'), nullable=False)
    
    gene = relationship('Gene', backref='names')
    
    def __repr__(self):
        return '<GeneName(id={!r}, name={!r})>'.format(self.id, self.name)

class Gene(DBEntity):
    
    '''
    name: Canonical name
    names: Canonical name and synonymous names (unordered)
    '''
    
    id =  Column(Integer, primary_key=True)
    description = Column(String(1000))
    canonical_name_id =  Column(Integer, ForeignKey('gene_name.id'), nullable=True)
    
    canonical_name = relationship('GeneName')  # The preferred name to assign to this gene
    # names = GeneName backref, all names
    
    def __repr__(self):
        return '<Gene(id={!r}, name={!r})>'.format(self.id, self.name)       
    
    
    
    
    
                                                  
