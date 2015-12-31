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

import sqlalchemy as sa
from sqlalchemy import create_engine, MetaData, Table
from sqlalchemy.orm import sessionmaker
import sqlalchemy.sql as sql
from deep_blue_genome.core.database.entities import LastId, DBEntity, Gene, GeneName,\
    GeneNameQueryItem
from deep_blue_genome.core.exceptions import TaskFailedException
from deep_blue_genome.core.exception_handlers import UnknownGeneHandler
from contextlib import contextmanager
import logging
import pandas as pd
import numpy as np
from more_itertools.more import chunked
from deep_blue_genome.util.pandas import df_has_null, df_count_null

_logger = logging.getLogger('deep_blue_genome.core.Database')
        
class Database(object):
    
    '''
    RDBMS support
    
    Note: Use bulk methods for large amounts or performance will suffer.
    
    None of Database's methods commit or rollback `Database.session` or any session passed into a method.
    
    Note: A Session object should not be shared across threads.
    '''
    
    def __init__(self, context, host, user, password, name): # TODO whether or not to add a session id other than "We are global, NULL, session"
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
        
        self._engine = create_engine('mysql+pymysql://{}:{}@{}/{}'.format(user, password, host, name), echo=False)
        self._create()
        
        self._Session = sessionmaker(bind=self._engine)
        self._session = self.create_session()
        
    def dispose(self):
        self._Session.close_all()
        self._engine.dispose()
        
    @contextmanager
    def scoped_session(self):
        '''
        Create a session context manager
        
        Commits on exit unless exception was raised; always closes.
        '''
        session = self.create_session()
        try:
            yield session
            session.commit()
        except:
            session.rollback()
            raise
        finally:
            session.close()
        
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
        with self.scoped_session() as session:
            for name in self._table_names:
                if name != 'last_id':
                    session.add(LastId(table_name=name, last_id=0))
        
    def create_session(self):
        '''
        Create new session of database
        
        Consider using `self.scoped_session` instead.
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
        Analog to `get_next_ids`, except return value is an int.
        '''
        return next(self.get_next_ids(table, 1))
    
    def get_next_ids(self, table, count):
        '''
        Get id to use for next insert in table.
        
        This immediately commits (in a separate session) to database.
        
        Parameters
        ----------
        table : Table or ORM entity
        count : int
            How many ids to reserve
        
        Returns
        -------
        iterable of int
            The next free ids.
        '''
        with self.scoped_session() as session:
            if hasattr(table, '__tablename__'):
                name = table.__tablename__
            else:
                name = table.name
            #TODO could we bring it down to 1 round trip? update followed by select. Should we? We need profiling for DB operations
            record = session.query(LastId).filter_by(table_name=name).one()
            start = record.last_id+1
            record.last_id += int(count)
            return iter(range(start, record.last_id+1))
            
    def _get_genes_by_name(self, query_id, names, session):
        stmt = (
            session
            .query(GeneNameQueryItem.row, GeneNameQueryItem.column, Gene) # row, col added as otherwise same Genes would fold into 1 in the return for some reason 
            .select_from(GeneNameQueryItem)
            .filter_by(query_id=query_id)
            .join(GeneName, GeneName.name==GeneNameQueryItem.name, isouter=True)
            .join(Gene, Gene.id == GeneName.gene_id, isouter=True)
            .order_by(GeneNameQueryItem.row, GeneNameQueryItem.column)
        )
        return pd.DataFrame(chunked((row[2] for row in stmt), len(names.columns)), columns=names.columns, index=names.index)
    
    # TODO map=False (apply gene_mapping if True), map_suffix1 (map gene_name.1 to gene_name). Note: Order is: get genes by name -> apply map_suffix1 -> handle unknown genes -> apply map
    def get_genes_by_name(self, names, session=None, unknown_gene_handler=None):
        '''
        Get genes by name (including synonyms, ignoring case).
        
        Can be thought of as `names.applymap(get_gene_by_name)`, except it is much
        faster, and get_gene_by_name does not exist.
        
        If a gene is not found and unknown_gene handling is configured to 'add', a
        gene will be added with the given name as canonical name. If the
        handling is set to 'fail', this event will raise a TaskFailedException.
        If the handling is set to 'ignore', it will be returned as NaN.
        
        Parameters
        ----------
        names : pandas.DataFrame(data=[[name : str]])
            Each cell of the DataFrame is a name of a gene to get.
        session
            Session to use. Uses `self.session` if None. Concurrent calls to
            this function with the same session (including None) are currently
            not supported.
        unknown_gene_handler
            If not None, overrides the context's UnknownGeneHandler.
            
        Returns
        -------
        pandas.DataFrame(data=[[gene : Gene]])
            Data frame with same index as `names` and each cell's value replaced
            with a Gene or NaN (in case the gene name was not found and unknown
            gene handler is 'ignore').
             
        Raises
        ------
        TaskFailedException
            If a gene was not found and handling is set to 'fail' 
        '''
        if not session:
            session = self.session
        
        _logger.debug('Querying up to {} genes by name'.format(names.size))
        
        query_id = self.get_next_id(GeneNameQueryItem)
        try:
            # Insert query data
            # XXX column axis apply followed by row axis apply followed by to_dict('split')['data'] might be faster
            # XXX to_dict() might be faster
            session.bulk_insert_mappings(GeneNameQueryItem, [
                {'row': row, 'column': column, 'name': values[column], 'query_id': query_id}
                for row, values in enumerate(names.itertuples(index=False, name=None))
                for column in range(len(names.columns))
            ])
            
            # Find genes by name
            genes = self._get_genes_by_name(query_id, names, session)
            
            # Handle unknown genes
            count_missing = df_count_null(genes)
            if count_missing:
                _logger.info('Input has up to {} genes missing from database'.format(count_missing))
                
                if not unknown_gene_handler:
                    unknown_gene_handler = self._context.configuration['exception_handlers']['unknown_gene']
                    
                if unknown_gene_handler == UnknownGeneHandler.add:
                    # Add missing genes
                    
                    select_missing_names_stmt = (
                        session.query(GeneNameQueryItem.name)
                        .distinct()
                        .filter_by(query_id=query_id)
                        .join(GeneName, GeneName.name == GeneNameQueryItem.name, isouter=True)
                        .filter(GeneName.id.is_(None))
                    )
                    
                    # Insert genes
                    stmt = (
                        Gene.__table__
                        .insert()
                        .from_select(['id'], 
                            session
                            .query(sa.null())
                            .select_entity_from(select_missing_names_stmt.subquery())
                        )
                    )
                    session.execute(stmt)
                    
                    # Insert gene names
                    stmt = (
                        GeneName.__table__
                        .insert()
                        .from_select(['gene_id', 'name'],
                            sql.select([sql.text('@row := @row + 1'), select_missing_names_stmt.subquery()])
                            .select_from(sql.text('(SELECT @row := last_insert_id() - 1) range_'))
                        )
                    )
                    result = session.execute(stmt)
                    _logger.info('Added {} missing genes to database'.format(result.rowcount))
                        
                    # Get genes again, with missing ones added
                    genes = self._get_genes_by_name(query_id, names, session)
                    assert not df_has_null(genes)  # TODO rm when verified enough
                elif unknown_gene_handler == UnknownGeneHandler.ignore:
                    pass  # ignore
                elif unknown_gene_handler == UnknownGeneHandler.fail:
                    raise TaskFailedException('Encountered {} unknown genes'.format(count_missing))
                else:
                    assert False
        finally:
            session.query(GeneNameQueryItem).filter_by(query_id=query_id).delete(synchronize_session=False)
            
        return genes
            
