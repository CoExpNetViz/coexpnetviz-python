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
Database entities (i.e. tables)
'''

from deep_blue_genome.core.database.dbms_info import mysql_innodb
from sqlalchemy.ext.declarative import declarative_base, declared_attr
from inflection import underscore
from sqlalchemy import Column, Integer, String, DateTime, ForeignKey, Table
from sqlalchemy.orm import relationship
from deep_blue_genome.util.constants import URL_MAX_LENGTH, PATH_MAX_LENGTH
from datetime import datetime

# Note: we rely on the fact that mysql innodb's default collation is case-insensitive and the charset is utf8


class DBEntity(object):
    @declared_attr
    def __tablename__(cls):
        return underscore(cls.__name__)

DBEntity = declarative_base(cls=DBEntity)


class LastId(DBEntity):
    
    '''
    Contains last id used per table
    '''
    
    table_name = Column(String(250), primary_key=True, autoincrement=False)
    last_id =  Column(Integer)

class CachedFile(DBEntity):
    
    id =  Column(Integer, primary_key=True, autoincrement=False)
    source_url = Column(String(min(URL_MAX_LENGTH, mysql_innodb.max_index_key_length_char)), unique=True, nullable=False)
    path = Column(String(PATH_MAX_LENGTH), nullable=False)  # absolute path to file in cache
    cached_at = Column(DateTime, nullable=False)
    expires_at = Column(DateTime, nullable=False)
    
    def __repr__(self):
        return (
            '<CachedFile(id={!r}, source_url={!r}, path={!r}, cached_at={!r}'
            ', expires_at={!r})>'.format(
                self.id, self.source_url, self.path,
                self.cached_at, self.expires_at
            )
        )
    
    @property
    def expired(self):
        return self.expires_at <= datetime.now()
    
    
class GeneName(DBEntity):
     
    id =  Column(Integer, primary_key=True, autoincrement=False)
    name = Column(String(250), unique=True, nullable=False)
    gene_id =  Column(Integer, ForeignKey('gene.id'), nullable=False)
     
    gene = relationship('Gene', backref='names', foreign_keys=[gene_id])
     
    def __repr__(self):
        return '<GeneName(id={!r}, name={!r})>'.format(self.id, self.name)
 
 
class Gene(DBEntity):
     
    '''
    name: Canonical name
    names: Canonical name and synonymous names (unordered)
    '''
     
    id =  Column(Integer, primary_key=True, autoincrement=False)
    description = Column(String(1000), nullable=True)
    canonical_name_id =  Column(Integer, ForeignKey('gene_name.id'), nullable=True)
     
    canonical_name = relationship('GeneName', foreign_keys=[canonical_name_id], post_update=True)  # The preferred name to assign to this gene
    # names = GeneName backref, all names
     
    def __repr__(self):
        return '<Gene(id={!r}, canonical_name={!r})>'.format(self.id, self.canonical_name)    
    
    
GeneExpressionMatrixTable = Table('gene_expression_matrix', DBEntity.metadata,
    Column('gene_id', Integer, ForeignKey('gene.id')),
    Column('expression_matrix_id', Integer, ForeignKey('expression_matrix.id'))
)


class ExpressionMatrix(DBEntity):
     
    id =  Column(Integer, primary_key=True, autoincrement=False)
    path = Column(String(PATH_MAX_LENGTH), nullable=False)
     
    genes = relationship("Gene", secondary=GeneExpressionMatrixTable)  # Genes whose expression was measured in the expression matrix
     
    def __repr__(self):
        return '<ExpressionMatrix(id={!r}, path={!r})>'.format(self.id, self.path)
    


GeneClusteringTable = Table('gene_clustering', DBEntity.metadata,
    Column('gene_id', Integer, ForeignKey('gene.id')),
    Column('clustering_id', Integer, ForeignKey('clustering.id'))
)


class Clustering(DBEntity):
     
    id =  Column(Integer, primary_key=True, autoincrement=False)
    path = Column(String(PATH_MAX_LENGTH), nullable=False)
     
    genes = relationship("Gene", secondary=GeneClusteringTable)  # Genes mentioned in the clustering
     
    def __repr__(self):
        return '<Clustering(id={!r}, path={!r})>'.format(self.id, self.path)
    
    
    