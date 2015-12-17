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
from contextlib import contextmanager
import logging

_logger = logging.getLogger('deep_blue_genome.core.database.database')
        
class Database(object):
    
    '''
    RDBMS support
    
    Note: Use bulk methods for large amounts or performance will suffer.
    
    None of Database's methods commit `session`.
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
        
        self._engine = create_engine('mysql+pymysql://{}:{}@{}/{}'.format(user, password, host, name))
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
            record.last_id += count
            return iter(range(start, record.last_id+1))
    
    def get_gene_by_name(self, name, session=None):
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
        if not session:
            session = self.session
        try:
            return (session
                .query(Gene)
                .join(GeneName, Gene.id == GeneName.gene_id)
                .filter(GeneName.name == name)
                .one()
            )
        except NoResultFound as ex:
            unknown_gene_handling = self._context.configuration['exception_handling']['unknown_gene']
            if unknown_gene_handling == UnknownGeneHandling.add:
                gene_name = GeneName(id=self.get_next_id(GeneName), name=name)
                gene = Gene(id=self.get_next_id(Gene), names=[gene_name], canonical_name=gene_name)
                session.add(gene)
                return gene
            else:
                ex = GeneNotFoundException(name, ex)
                if unknown_gene_handling == UnknownGeneHandling.ignore:
                    raise ex
                elif unknown_gene_handling == UnknownGeneHandling.fail:
                    raise TaskFailedException(cause=ex)
                else:
                    assert False
                    
    def get_genes_by_name(self, names, session=None):
        '''
        Get genes by name (including synonyms, ignoring case).
        
        Faster than `get_gene_by_name` unless you need only look up a single gene.
        
        If a gene is not found and unknown_gene handling is configured to 'add', a
        gene will be added with the given name as canonical name. If the
        handling is set to 'fail', this event will raise a TaskFailedException.
        If the handling is set to 'ignore', it will be returned in the missing list.
        
        Parameters
        ----------
        names : iterable of str
            Names of genes to fetch. If the same gene is mentioned more than
            once, it may be present in the return more than once (this includes
            synonyms).
        session
            Session to use. Uses `self.session` if None.
            
        Returns
        -------
        (genes : list of Gene, missing : list of str)
            Returns the genes (including added genes if handling is 'add'), and
            names of genes not found (if handling is 'ignore').
             
        Raises
        ------
        TaskFailedException
            If a gene was not found and handling is set to 'fail' 
        '''
        _logger.debug('get_genes_by_name')
        names = set(names)
        if not session:
            session = self.session
        
        # Select unknown gene names
        names_present = list(
            session
            .query(GeneName)
            .filter(GeneName.name.in_(names))
        )
        missing = names - {r.name for r in names_present}
        
        # Get present genes
        if names_present:
            _logger.debug('Getting {} present genes'.format(len(names_present)))
            genes = list(
                session
                .query(Gene)
                .filter(Gene.id.in_([name.gene_id for name in names_present]))
            )
        else:
            genes = []
        
        # Handle unknown genes
        if missing:
            _logger.warn('Encountered {} unknown genes'.format(len(missing)))
            unknown_gene_handling = self._context.configuration['exception_handling']['unknown_gene']
            if unknown_gene_handling == UnknownGeneHandling.add:
                _logger.debug('Adding unknown genes')
                next_gene_name_ids = self.get_next_ids(GeneName, len(missing))
                next_gene_ids = self.get_next_ids(Gene, len(missing))
                for name in missing:
                    gene_name = GeneName(id=next(next_gene_name_ids), name=name)
                    gene = Gene(id=next(next_gene_ids), names=[gene_name], canonical_name=gene_name)
                    session.add(gene)
                    genes.append(gene)
                missing = []
            elif unknown_gene_handling == UnknownGeneHandling.ignore:
                pass  # ignore
            elif unknown_gene_handling == UnknownGeneHandling.fail:
                raise TaskFailedException(cause=GeneNotFoundException(missing[0]))
            else:
                assert False
            
        return genes, missing
            
