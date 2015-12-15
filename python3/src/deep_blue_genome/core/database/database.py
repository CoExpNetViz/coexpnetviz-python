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

from sqlalchemy import create_engine, MetaData, Table
from sqlalchemy.orm import sessionmaker
from deep_blue_genome.core.database.entities import LastId, DBEntity

class GeneNotFoundException(Exception):
    def __init__(self, gene_name, ex=None):
        super().__init__(self, "Could not find gene with name '{}'. Cause: {}".format(gene_name, ex))
        
class NotFoundException(Exception):
    def __init__(self, name, ex=None):
        super().__init__(self, "Could not find {}. Cause: {}".format(name, ex))
        
class Database(object):
    
    '''
    RDBMS support
    
    Note: Use bulk methods for large amounts or performance will suffer.
    '''
    
    def __init__(self, host, user, password, name):
        '''
        Create instance connected to MySQL
        
        Parameters
        ----------
        host : str
            dns or ip of the DB host
        user : str
            username to log in with
        password : str
            password to log in with
        name : str
            name of database
        ''' 
        self._engine = create_engine('mysql+pymysql://{}:{}@{}/{}'.format(user, password, host, name))
        self._create()
        
        self._Session = sessionmaker()
        self._session = self.create_session()
        self._last_id_session = self.create_session()
        '''
        Session for incrementing an id in NextId table; wouldn't want an entire
        transaction to rollback due to a simple race conflict on getting the
        next id for a table
        '''
        
    def commit(self):
        self._session.commit()
        
    def _create(self):
        '''
        Create missing tables.
        '''
        DBEntity.metadata.create_all(self._engine)
        
    @property
    def _table_names(self):
        '''list of table names'''
        return [row[0] for row in self._session.execute('show tables')]
        
    def recreate(self):
        '''
        Recreate everything: tables, ...
        
        This is a very destructive operation that clears everything (including
        the data) from the database. Use with care.
        '''
        # Drop tables
        metadata = MetaData(bind=self._engine)
        for name in self._table_names:
            Table(name, metadata, autoload=True, autoload_with=self._engine)
        metadata.drop_all()
        
        # Create everything
        self._create()
        
        # Init next ids for tables
        for name in self._table_names:
            self._last_id_session.add(LastId(table_name=name, last_id=0))
        self._last_id_session.commit()
        
    def create_session(self):
        '''
        Create new session of database
        '''
        return self._Session(bind=self._engine)
    
    @property
    def session(self):
        '''
        Application global SQLAlchemy database session.
        '''
        return self._session
    
#     def get_gene_by_name(self, name):
#         '''
#         Get gene by name (including synonyms, ignoring case)
#         
#         Returns
#         -------
#         Gene
#             The gene found
#             
#         Raises
#         ------
#         GeneNotFoundError
#         '''
#         try:
#             return self._session.query(Gene).\
#                         join(GeneName, Gene.name).\
#                         filter(func.lower(GeneName.name) == func.lower(name)).\ #TODO lower is slow, also, use ()
#                         one()
#         except NoResultFound as ex:
#             raise GeneNotFoundException(name, ex)

    def get_next_id(self, table):
        '''
        table : Table or ORM entity
        '''
        if hasattr(table, '__tablename__'):
            name = table.__tablename__
        else:
            name = table.name
        record = self._last_id_session.query(LastId).filter_by(table_name=name).one()
        record.last_id += 1
        self._last_id_session.commit()
        return record.last_id
        
