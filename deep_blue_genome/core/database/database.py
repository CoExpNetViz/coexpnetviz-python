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
from sqlalchemy.orm import sessionmaker, aliased
import sqlalchemy.sql as sql
from deep_blue_genome.core.database.entities import LastId, DBEntity, Gene, GeneName,\
    GeneNameQueryItem, ExpressionMatrix, BaitsQueryItem,\
    Clustering, GeneExpressionMatrixTable, GeneMappingTable, GeneClusteringTable
from deep_blue_genome.core.exceptions import TaskFailedException
from deep_blue_genome.core.exception_handlers import UnknownGeneHandler
from contextlib import contextmanager
import logging
import pandas as pd
import numpy as np
from chicken_turtle_util.pandas import df_count_null
import re
from collections import namedtuple
from chicken_turtle_util.debug import print_sql_stmt
from deep_blue_genome.core.reader.various import read_expression_matrix_file

_logger = logging.getLogger('deep_blue_genome.core.Database')

_ReturnTuple = namedtuple('_ReturnTuple', 'expression_matrices clusterings'.split())

class Database(object):
    
    '''
    RDBMS support
    
    Any `load_*` function, loads additional data on an entity. Allowing to load
    lazy relations and deferred columns in bulk.
    
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
        
        self._engine = sa.create_engine('mysql+pymysql://{}:{}@{}/{}'.format(user, password, host, name), echo=False)
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
        metadata = sa.MetaData(bind=self._engine)
        for name in self._table_names:
            sa.Table(name, metadata, autoload=True, autoload_with=self._engine)
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
        
    def _get_genes_by_name(self, query_id, names, session, map_): # XXX untested
        '''Quite coupled to get_genes_by_name'''
            
        # Build query
        stmt = (  # Select existing genes by name
            session
            .query() 
            .select_from(GeneNameQueryItem)
            .filter_by(query_id=query_id)
            .join(GeneName, GeneNameQueryItem.name == GeneName.name)
            .join(Gene, GeneName.gene)
        )
        
        if isinstance(names, pd.Series):
            if map_:
                MappedGene = aliased(Gene)
                
                # Select the MappedGene.id if exists, otherwise select Gene.id
                sub = (
                    stmt
                    .with_entities(GeneNameQueryItem.row, sql.func.ifnull(MappedGene.id, Gene.id).label('gene_id'))
                    .join(MappedGene, Gene.mapped_to, isouter=True)
                    .subquery()
                )
                
                # Select Gene instead of id
                stmt = (
                    session
                    .query(sub.c.row, Gene)
                    .select_from(Gene)
                    .join(sub, Gene.id == sub.c.gene_id)
                ) 
            else:
                stmt = stmt.with_entities(GeneNameQueryItem.row, Gene)
        else:
            stmt = stmt.with_entities(GeneNameQueryItem.row, GeneNameQueryItem.column, Gene)
        
        # Load result
        if isinstance(names, pd.Series):
            genes = pd.DataFrame(iter(stmt), columns=['row', 'value'])
            genes.set_index('row', inplace=True)
            genes = genes['value']
            genes.rename(dict(enumerate(names.index)), inplace=True)
            if not map_:
                genes = genes.reindex(index=names.index)
            genes.name = names.name
            genes.index.name = names.index.name
        else:
            genes = pd.DataFrame(iter(stmt), columns=['row', 'column', 'value']).pivot(index='row', columns='column', values='value')
            genes.rename(
                index=dict(enumerate(names.index)),
                columns=dict(enumerate(names.columns)),
                inplace=True
            )
            genes = genes.reindex(index=names.index, columns=names.columns)
        
        return genes
     
    def get_genes_by_name(self, names, session=None, unknown_gene_handler=None, map_=False, map_suffix1=False): #XXX more examples in docstring: 1 for each return case
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
        unknown_gene_handler : UnknownGeneHandler
            If not None, override the context.configuration.unknown_gene_handler.
        map_suffix1 : bool
            Whether or not to map 'gene_name.1' to 'gene_name'
        map_ : bool
            Whether or not to map genes by their gene mapping (from left- to
            right-hand of mapping). Cannot be True if `names` is a DataFrame.
            
        Returns
        -------
        pandas.DataFrame(data=[[_ : Gene]], index=names.index, columns=names.columns)
            If `names` is a DataFrame, a data frame is returned with the same
            index as `names` and each cell's value replaced with a Gene or NaN
            (in case the gene name was not found and unknown gene handler is
            'ignore').
            
        pandas.Series(data=[names.name : Gene], index=names.index)
            If `names` is a Series and not `map_`, a series is returned with
            same index as `names` and each cell's value replaced with a Gene or
            NaN (in case the gene name was not found and unknown gene handler is
            'ignore').
            
        pandas.Series(data=[names.name : Gene])
            If `names` is a Series and `map_`, a series is returned with an
            index based on `names.index`. The index is a subset of `names.index`
            where elements are repeated if the corresponding gene was mapped to
            multiple genes (by map_), and omitted if they weren't found and the
            unknown gene handler is 'ignore'.
             
        Raises
        ------
        TaskFailedException
            If a gene was not found and handling is set to 'fail' 
        '''
        if not session:
            session = self.session
            
        if not unknown_gene_handler:
            unknown_gene_handler = self._context.configuration.unknown_gene_handler
            
        if isinstance(names, pd.DataFrame) and map_:
            raise ValueError('map_=True not available for DataFrame input')
        
        _logger.debug('Querying up to {} genes by name'.format(names.size))
        
        query_id = self.get_next_id(GeneNameQueryItem)
        try:
            # Apply map_suffix1, maybe
            if map_suffix1:
                names = names.applymap(lambda x: re.sub(r'\.1$', '', x))
            
            # Insert query data
            names_indices = names.copy(deep=False)
            names_indices.index = range(len(names.index))
            names_indices.index.name = 'row'
            if isinstance(names, pd.Series):
                names_indices.name = 'name'
                names_indices = names_indices.reset_index()
                names_indices['column'] = 0
            else:
                names_indices.columns = range(len(names.columns))
                names_indices = names_indices.reset_index()
                names_indices = pd.melt(names_indices, id_vars='row', var_name='column', value_name='name')
            names_indices['query_id'] = query_id
            session.bulk_insert_mappings(GeneNameQueryItem, names_indices.to_dict('record'))
            
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
    
    def _get_gene_collections_by_genes(self, query_id, min_genes_present, GeneCollection, gene_to_collection_relation, gene_collection_name, session=None):
        '''
        Highly coupled to `get_gene_collections_by_genes`
        '''
        
        # Match baits to matrix genes
        select_baits_unmapped_container_stmt = (
            session
            .query(BaitsQueryItem.baits_id.label('baits_id'), Gene, GeneCollection)
            .select_from(BaitsQueryItem)
            .filter_by(query_id=query_id)
            .join(Gene)
            .join(GeneCollection, gene_to_collection_relation)
        )
        
        # Match baits to mapped matrix genes
        MappedGene = aliased(Gene, name='mapped_gene')
        select_baits_mapped_container_stmt = (
            session
            .query(BaitsQueryItem.baits_id.label('baits_id'), Gene, GeneCollection)
            .select_from(BaitsQueryItem)
            .filter_by(query_id=query_id)
            .join(MappedGene)
            .join(Gene, MappedGene.mapped_from)
            .join(GeneCollection, gene_to_collection_relation)
        )
        
        # Union previous 2 selects
        select_baits_container_stmt = select_baits_unmapped_container_stmt.union_all(select_baits_mapped_container_stmt)
        
        # Filter (baits_id, exp mat) combos to those with enough baits in them
        entities = [BaitsQueryItem.baits_id, GeneCollection.id.label('collection_id')]
        bait_count_filter_stmt = (
            select_baits_container_stmt
            .with_entities(*entities)
            .group_by(*entities)
            .having(sql.func.count() >= min_genes_present)
        )
        
        # Select the whole deal, with above filterdy 
        filter_sub = bait_count_filter_stmt.subquery(name='filter_sub')
        stmt = (
            select_baits_container_stmt
            .with_entities(BaitsQueryItem.baits_id, Gene, GeneCollection)
            .join(filter_sub, sql.and_(
                GeneCollection.id == filter_sub.c.collection_id,
                BaitsQueryItem.baits_id == filter_sub.c.baits_id,
            ))
        )
        
        # Run query and return result
        return pd.DataFrame(iter(stmt), columns=['group_id', 'gene', gene_collection_name])
        
    # XXX rewrite docstrings: pd.DataFrame(...) -> pandas.DataFrame({'key' : [val_type]})
    def get_gene_collections_by_genes(self, gene_groups, min_genes_present, expression_matrices=False, clusterings=False, session=None):
        '''
        Get expression matrices and/or clusterings containing (some of) given genes
        
        `gene_groups` is compared to mapped genes of expression_matrices.
        `gene_groups` are expected to have already been mapped.
            
        Parameters
        ----------
        gene_groups : pd.DataFrame(columns=[group_id : int, gene : Gene])
            list of gene collections to which non-bait genes are compared
        min_genes_present : int
            Minimum number of genes present in TODO
        map_ : bool
            If True, 
        expression_matrices : bool
        clusterings : bool
            
        Returns
        -------
        namedtuple of (
            expression_matrices = pd.DataFrame(columns=[group_id : int, gene : Gene, expression_matrix : ExpressionMatrix]),
            clusterings = pd.DataFrame(columns=[group_id : int, gene : Gene, clustering : Clustering]),
        )
            If `expression_matrices`, baits present in expression matrix will be
            returned in `tuple.expression_matrices`. The analog is true for
            `clusterings`. The returned `gene` values are the genes found in the
            expression matrix or clustering (i.e. without any mapping applied).
        '''
        if not session:
            session = self.session
            
        # Insert baitss
        query_id = self.get_next_id(BaitsQueryItem)
        gene_groups = gene_groups.copy()
        gene_groups['query_id'] = query_id
        gene_groups['gene'] = gene_groups['gene'].apply(lambda x: x.id)
        gene_groups.rename(columns={'group_id': 'baits_id', 'gene': 'bait_id'}, inplace=True) #XXX rename entity to group_id, gene_id
        try:
            session.bulk_insert_mappings(BaitsQueryItem, gene_groups.to_dict('record')) # XXX this should always be undone, so when thrown out the session should be rolled back, but we're not allowed to mess with `session`. So, we should use a separate session for this and rm rows from it regardless and then commit that (don't rollback, that may cause rollback avalanches)
            
            if expression_matrices:
                expression_matrices = self._get_gene_collections_by_genes(query_id, min_genes_present, ExpressionMatrix, Gene.expression_matrices, 'expression_matrix', session)
            else:
                expression_matrices = None
            
            if clusterings:
                clusterings = self._get_gene_collections_by_genes(query_id, min_genes_present, Clustering, Gene.clusterings, 'clustering', session)
            else:
                clusterings = None
                
        finally:  # XXX used this twice, make DRY
            session.query(BaitsQueryItem).filter_by(query_id=query_id).delete(synchronize_session=False)
            
        return _ReturnTuple(expression_matrices=expression_matrices, clusterings=clusterings)
    
    # XXX rm or needed?
#     def load_gene_details(self, genes, names=False, session=None):
#         '''
#         Load more detailed info for given genes
#         
#         genes : pd.DataFrame([[Gene]])
#         names : bool
#             Whether or not to load names and canonical name of gene
#         '''
#         if not session:
#             session = self.session
#         
#         if names:
#             stmt = (
#                 session
#                 .query(Gene)
#                 # TODO load names and canonical_name http://docs.sqlalchemy.org/en/latest/orm/loading_relationships.html
#             )
#             print_sql_stmt(stmt)
#             stmt.all()
#         # TODO test this:
#         # When a Gene is loaded in the same session, it will be the same object. So loading additional stuff, should load it on the relevant objects. So no need for any assignments or returns.

    def get_expression_matrix_data(self, expression_matrix, session=None):
        '''
        Get expression matrix data by meta data
        
        Parameters
        ----------
        expression_matrix : database.ExpressionMatrix
            Meta data of the expression matrix
            
        Returns
        -------
        pandas.DataFrame({condition_name : [float]}, index=('gene' : [Gene]))
        '''
        if not session:
            session = self._session
            
        expression_matrix_ = read_expression_matrix_file(expression_matrix.path)
            
        # Swap gene names for actual genes
        matrix_genes = self.get_genes_by_name(expression_matrix_.index.to_series(), map_=True)
        expression_matrix_ = expression_matrix_.reindex(matrix_genes.index)
        expression_matrix_.index = matrix_genes
        assert not expression_matrix_.index.has_duplicates  # currently assuming this never happens  # XXX enforce things in gene mapping loading so it indeed can't happen
        
        return expression_matrix_
        
        