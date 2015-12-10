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

from deep_blue_genome.core.exceptions import GeneNotFoundException
from deep_blue_genome.core.database.dbentity import DBEntity
from deep_blue_genome.core.database.gene import GeneName, Gene
from sqlalchemy import create_engine, func, MetaData, Table
from sqlalchemy.orm import sessionmaker
from sqlalchemy.orm.exc import NoResultFound

# DBMSInfo = namedtuple('DBMSInfo', 'max_bind_parameters'.split())
# sqlite_innodb_info = DBMSInfo(max_bind_parameters=100)

class Database(object):
    
    '''
    Encapsulates all data access
    
    Abstracts how data is stored (RDBMS, NoSQL, ...).
    
    Use bulk methods for large amounts or performance will suffer.
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
        # TODO debug only: rm db
        from plumbum import local
        db = local.path('main.db')
        if db.exists():
            db.delete()
            
        self._engine = create_engine('mysql+pymysql://{}:{}@{}/{}'.format(user, password, host, name), echo=True)
        self._create()
        
        Session = sessionmaker()
        self._session = Session(bind=self._engine)
        
    def commit(self):
        self._session.commit()
        
    def _create(self):
        '''
        Create missing tables.
        '''
        DBEntity.metadata.create_all(self._engine)
        
    def recreate(self):
        '''
        Recreate everything: tables, ...
        
        This is a very destructive operation that clears everything (including
        the data) from the database. Use with care.
        '''
        # Drop tables
        metadata = MetaData(bind=self._engine)
        for row in self._session.execute('show tables'):
            Table(row[0], metadata, autoload=True, autoload_with=self._engine)
        metadata.drop_all()
        
        # Create everything
        self._create()
        
    @property
    def session(self):
        '''
        Underlying SQLAlchemy database session.
        
        Use this only when you have to; e.g. when doing bulk operations. Otherwise use this class' interface.
        '''
        return self._session
    
    def get_gene_by_name(self, name):
        '''
        Get gene by name (including synonyms, ignoring case)
        
        Returns
        -------
        Gene
            The gene found
            
        Raises
        ------
        GeneNotFoundError
        '''
        try:
            return self._session.query(Gene).\
                        join(GeneName, Gene.name).\
                        filter(func.lower(GeneName.name) == func.lower(name)).\
                        one()
        except NoResultFound as ex:
            raise GeneNotFoundException(name, ex)
    
    


