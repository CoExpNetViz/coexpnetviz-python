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
from sqlalchemy.orm.exc import NoResultFound
from deep_blue_genome.core.database.entities import LastId, DBEntity, Gene, GeneName
from deep_blue_genome.util.debug import log_sql
from deep_blue_genome.core.exceptions import TaskFailedException,\
    GeneNotFoundException
from deep_blue_genome.core.exception_handling import UnknownGeneHandling
        
class Database(object):
    
    '''
    RDBMS support
    
    Note: Use bulk methods for large amounts or performance will suffer.
    '''
    
    def __init__(self, context, host, user, password, name):
        '''
        Create instance connected to MySQL
        
        Parameters
        ----------
        context : ConfigurationMixin
        host : str
            dns or ip of the DB host
        user : str
            username to log in with
        password : str
            password to log in with
        name : str
            name of database
        ''' 
        self._context = context
        
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
        
    def dispose(self):
        self._Session.close_all()
        self._engine.dispose()
        
    def commit(self):
        self._session.commit()
        
    def _create(self):
        '''
        Create missing tables.
        '''
        DBEntity.metadata.create_all(self._engine)
        
    @property
    def _table_names(self): # TODO engine.table_names
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

    def get_next_id(self, table):
        '''
        Get id to use for next insert in table
        
        Parameters
        ----------
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
    
    def get_gene_by_name(self, name):
        '''
        Get gene by name (including synonyms, ignoring case).
        
        If Gene is not found and unknown_gene handling is configured to 'add', a
        gene will be added with the given name as canonical name. If the
        handling is set to 'fail', this event will raise a TaskFailedException.
        If the handling is set to 'ignore', GeneNotFoundException is raised.
        
        Parameters
        ----------
        name : str
            One of the Gene's names
            
        Returns
        -------
        Gene
            The found/added gene
             
        Raises
        ------
        GeneNotFoundException
            If gene not found and unknown_gene handling is set to 'ignore'.
        TaskFailedException
        '''
        try:
            with log_sql():
                return (self._session
                    .query(Gene)
                    .join(GeneName, Gene.id == GeneName.gene_id)
                    .filter(GeneName.name == name)
                    .one()
                )
        except NoResultFound as ex:
            unknown_gene_handling = self._context.configuration['exception_handling']['unknown_gene']
            if unknown_gene_handling == UnknownGeneHandling.add:
                with log_sql():
                    gene_name = GeneName(id=self.get_next_id(GeneName), name=name)
                    gene = Gene(id=self.get_next_id(Gene), names=[gene_name], canonical_name=gene_name)
                    self._session.add(gene)
                    self._session.commit()
                return gene
            else:
                ex = GeneNotFoundException(name, ex)
                if unknown_gene_handling == UnknownGeneHandling.ignore:
                    raise ex
                elif unknown_gene_handling == UnknownGeneHandling.fail:
                    raise TaskFailedException(cause=ex)
                else:
                    assert False
                    
