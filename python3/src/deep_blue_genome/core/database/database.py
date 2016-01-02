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

import sqlalchemy as sa
from sqlalchemy import create_engine, MetaData, Table
from sqlalchemy.orm import sessionmaker
import sqlalchemy.sql as sql
from deep_blue_genome.core.database.entities import LastId, DBEntity, Gene, GeneName,\
    GeneNameQueryItem, GeneMapping, ExpressionMatrix, BaitsQueryItem,\
    Clustering
from deep_blue_genome.core.exceptions import TaskFailedException
from deep_blue_genome.core.exception_handlers import UnknownGeneHandler
from contextlib import contextmanager
import logging
import pandas as pd
import numpy as np
from deep_blue_genome.util.pandas import df_count_null
import re
from collections import namedtuple

_logger = logging.getLogger('deep_blue_genome.core.Database')

_ReturnTuple = namedtuple('_ReturnTuple', 'expression_matrices clusterings'.split())
        
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
            # XXX could we bring it down to 1 round trip? update followed by select. Should we? We need profiling for DB operations
            record = session.query(LastId).filter_by(table_name=name).one()
            start = record.last_id+1
            record.last_id += int(count)
            return iter(range(start, record.last_id+1))
            
            
    def _add_unknown_genes(self, query_id, session):
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
        unknown_genes_count = session.execute(stmt).rowcount
        
        # Continue only if there actually were unknown genes 
        if unknown_genes_count:
            # Insert gene names
            stmt = (
                GeneName.__table__
                .insert()
                .from_select(['gene_id', 'name'],
                    sql.select([sql.text('@row := @row + 1'), select_missing_names_stmt.subquery()])
                    .select_from(sql.text('(SELECT @row := last_insert_id() - 1) range_'))
                )
            )
            session.execute(stmt)
            _logger.info('Added {} missing genes to database'.format(unknown_genes_count))
            
    def _df_from_result(self, result):
        '''
        Get DataFrame from (row, column, value) iterable
        
        result : iterable of (row, column, value)
        '''
        return pd.DataFrame(iter(result), columns=['row', 'column', 'value']).pivot(index='row', columns='column', values='value')
#         return pd.DataFrame(chunked((row[2] for row in result), len(columns)), columns=columns, index=index) # XXX If slow, you could try this instead. It requires ordered by row, col and a None for every missing val
        
    def _df_restore_axi_from(self, df, target):
        '''
        Restore `df`'s columns and index from `target` by treating current names
        as indices into `target`'s axi
        '''
        df = df.rename(
            index=dict(zip(range(len(target.index)), target.index)),
            columns=dict(zip(range(len(target.columns)), target.columns)),
        )
        df = df.reindex(index=target.index, columns=target.columns)
        return df
        
    def _get_genes_by_name(self, query_id, names, session, map_): # XXX untested
        '''Coupled to get_genes_by_name'''
        # Select unmapped genes
        stmt = (
            session
            .query(GeneNameQueryItem.row, GeneNameQueryItem.column, Gene) # row, col added as otherwise same Genes would fold into 1 in the return for some reason 
            .select_from(GeneNameQueryItem)
            .filter_by(query_id=query_id)
            .join(GeneName, GeneNameQueryItem.name == GeneName.name)
        )
        select_genes_stmt = stmt.join(Gene, Gene.id == GeneName.gene_id)
        genes = self._df_from_result(select_genes_stmt)
        genes = self._df_restore_axi_from(genes, names)
        
        # Select mapped genes
        if map_:
            select_mapped_genes_stmt = (
                stmt
                .join(GeneMapping, GeneName.gene_id == GeneMapping.left_id)
                .join(Gene, GeneMapping.right_id == Gene.id)
            )
            mapped_genes = self._df_from_result(select_mapped_genes_stmt)
            mapped_genes = self._df_restore_axi_from(mapped_genes, names)
            
            # Override unmapped with mapped
            mapped_genes.fillna(genes, inplace=True)
            genes = mapped_genes
        
        return genes
     
    def get_genes_by_name(self, names, session=None, unknown_gene_handler=None, map_=False, map_suffix1=False):
        '''
        Get genes by name (including synonyms, ignoring case).
        
        Can be thought of as `names.applymap(get_gene_by_name)`, except it is much
        faster, and get_gene_by_name does not exist.
        
        If a gene is not found and unknown_gene handling is configured to 'add', a
        gene will be added with the given name as canonical name. If the
        handling is set to 'fail', this event will raise a TaskFailedException.
        If the handling is set to 'ignore', it will be returned as NaN.
        
        First each gene_name.1 is mapped to gene_name, if `map_suffix1`. Then
        missing genes are added if the handler is `UnknownGeneHandler.add`.
        Finally, gene mappings are applied if `map_` is True.
        
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
        map_suffix1 : bool
            Whether or not to map 'gene_name.1' to 'gene_name'
        map_ : bool
            Whether or not to map genes by their gene mapping (from left- to right-hand of mapping)
            
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
            
        if not unknown_gene_handler:
            unknown_gene_handler = self._context.configuration.unknown_gene_handler
        
        _logger.debug('Querying up to {} genes by name'.format(names.size))
        
        query_id = self.get_next_id(GeneNameQueryItem)
        try:
            # Apply map_suffix1, maybe
            if map_suffix1:
                names = names.applymap(lambda x: re.sub(r'\.1$', '', x)) # TODO check
            
            # Insert query data
            # XXX column axis apply followed by row axis apply followed by to_dict('split')['data'] might be faster
            # XXX to_dict() might be faster
            session.bulk_insert_mappings(GeneNameQueryItem, [
                {'row': row, 'column': column, 'name': values[column], 'query_id': query_id}
                for row, values in enumerate(names.itertuples(index=False, name=None))
                for column in range(len(names.columns))
            ])
            
            # Add unknown genes, maybe
            if unknown_gene_handler == UnknownGeneHandler.add:
                self._add_unknown_genes(query_id, session)
            
            # Find genes by name
            genes = self._get_genes_by_name(query_id, names, session, map_=map_)
            
            # Handle unknown genes
            count_missing = df_count_null(genes)
            if count_missing:
                _logger.info('Input has up to {} genes not known to the database'.format(count_missing))    
                if unknown_gene_handler == UnknownGeneHandler.ignore:
                    pass  # ignore
                elif unknown_gene_handler == UnknownGeneHandler.fail:
                    raise TaskFailedException('Encountered {} unknown genes'.format(count_missing))
                else:
                    assert False
        finally:
            session.query(GeneNameQueryItem).filter_by(query_id=query_id).delete(synchronize_session=False)
            
        return genes
    
    def _get_expression_matrices_by_genes(self, query_id, min_genes_present, session=None):
        base_stmt = (
            session
            .query(BaitsQueryItem)
            .filter_by(query_id=query_id)
            .join(Gene)
            .join(ExpressionMatrix, Gene.expression_matrices)
        )
        
        filtered_matrices_sub = (
            base_stmt
            .with_entities(BaitsQueryItem.baits_id, ExpressionMatrix.id)
            .group_by(BaitsQueryItem.baits_id, ExpressionMatrix.id)
            .having(sql.func.count() >= min_genes_present)
            .subquery()
        )
        
        stmt = (
            base_stmt
            .with_entities(BaitsQueryItem.baits_id, Gene, ExpressionMatrix)
            .join(filtered_matrices_sub, sql.and_( 
                ExpressionMatrix.id == filtered_matrices_sub.c.id,
                BaitsQueryItem.baits_id == filtered_matrices_sub.c.baits_id,
            ))
        )
        
        expression_matrices = pd.DataFrame(iter(stmt), columns=['baits_id', 'bait', 'expression_matrix'])
        return expression_matrices
    
    def _get_clusterings_by_genes(self, query_id, min_genes_present, session=None):
        base_stmt = (
            session
            .query(BaitsQueryItem)
            .filter_by(query_id=query_id)
            .join(Gene)
            .join(Clustering, Gene.clusterings)
        )
        
        filtered_matrices_sub = (
            base_stmt
            .with_entities(BaitsQueryItem.baits_id, Clustering.id)
            .group_by(BaitsQueryItem.baits_id, Clustering.id)
            .having(sql.func.count() >= min_genes_present)
            .subquery()
        )
        
        stmt = (
            base_stmt
            .with_entities(BaitsQueryItem.baits_id, Gene, Clustering)
            .join(filtered_matrices_sub, sql.and_( 
                Clustering.id == filtered_matrices_sub.c.id,
                BaitsQueryItem.baits_id == filtered_matrices_sub.c.baits_id,
            ))
        )
        
        clusterings = pd.DataFrame(iter(stmt), columns=['baits_id', 'bait', 'clustering'])
        return clusterings
        
    def get_by_genes(self, geness, min_genes_present, expression_matrices=False, clusterings=False, session=None):
        '''
        Parameters
        ----------
        geness : pd.DataFrame(columns=[baits_id : int, bait : Gene])
            list of gene collections to which non-bait genes are compared
        min_genes_present : int
            Minimum number of genes present in TODO
        expression_matrices : bool
        clusterings : bool
            
        Returns
        -------
        namedtuple of (
            expression_matrices = pd.DataFrame(columns=[baits_id : int, bait : Gene, expression_matrix : ExpressionMatrix]),
            clusterings = pd.DataFrame(columns=[baits_id : int, bait : Gene, clustering : Clustering]),
        )
            If `expression_matrices`, baits present in expression matrix will be
            returned in `tuple.expression_matrices`. The analog is true for
            `clusterings`.
        '''
        if not session:
            session = self.session
            
        # Insert baitss
        query_id = self.get_next_id(BaitsQueryItem)
        baitss = geness.copy()
        baitss['query_id'] = query_id
        baitss['bait'] = baitss['bait'].apply(lambda x: x.id)
        baitss.rename(columns={'bait': 'bait_id'}, inplace=True)
        try:
            session.bulk_insert_mappings(BaitsQueryItem, baitss.to_dict('record')) # XXX this should always be undone, so when thrown out the session should be rolled back, but we're not allowed to mess with `session`. So, we should use a separate session for this and rm rows from it regardless and then commit that (don't rollback, that may cause rollback avalanches)
            
            if expression_matrices:
                expression_matrices = self._get_expression_matrices_by_genes(query_id, min_genes_present, session)
            else:
                expression_matrices = None
            
            if clusterings:
                clusterings = self._get_clusterings_by_genes(query_id, min_genes_present, session)
            else:
                clusterings = None
                
        finally:  # XXX used this twice, make DRY
            session.query(BaitsQueryItem).filter_by(query_id=query_id).delete(synchronize_session=False)
            
        return _ReturnTuple(expression_matrices=expression_matrices, clusterings=clusterings)
