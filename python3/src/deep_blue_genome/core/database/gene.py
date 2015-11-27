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
from sqlalchemy import Column, Integer, String, ForeignKey, ForeignKeyConstraint, Table
from sqlalchemy.orm import relationship
from sqlalchemy.sql.schema import PrimaryKeyConstraint
    
_gene_canonical_name_table = Table('gene_canonical_name', DBEntity.metadata,
    Column('gene_id', Integer, nullable=False),
    Column('name_id', Integer, nullable=False),
    PrimaryKeyConstraint('gene_id', 'name_id'),
    ForeignKeyConstraint(['gene_id', 'name_id'],['gene_name.gene_id', 'gene_name.id'])
)
'''1-to-1 association of canonical name to gene via secondary table to avoid cyclic dependency trouble'''
    
class GeneName(DBEntity):
    
    id =  Column(Integer, primary_key=True)
    name = Column(String, unique=True, nullable=False)
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
    ncbi_id = Column(Integer, unique=True) 
    description = Column(String)
    
    name = relationship('GeneName', uselist=False, secondary=_gene_canonical_name_table, 
                primaryjoin= id == _gene_canonical_name_table.c.gene_id,
                secondaryjoin= GeneName.id == _gene_canonical_name_table.c.name_id)
    # names = GeneName backref
    
    def __repr__(self):
        return '<Gene(id={!r}, name={!r})>'.format(self.id, self.name)       
    
    
    
    
    
                                                  
