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
from sqlalchemy import create_engine, func
from sqlalchemy.orm import sessionmaker
from sqlalchemy.orm.exc import NoResultFound

class Database(object):
    
    '''
    Encapsulates all data access
    
    Abstracts how data is stored (RDBMS, NoSQL, ...).
    
    Use bulk methods for large amounts or performance will suffer.
    '''
    
    def __init__(self):
        # TODO debug only: rm db
        from plumbum import local
        db = local.path('main.db')
        if db.exists():
            db.delete()
            
        engine = create_engine('sqlite:///main.db', echo=True)
        #TODO Probably something that data prep tool should do?
        DBEntity.metadata.create_all(engine)
        
        Session = sessionmaker()
        self._session = Session(bind=engine)
        self.session = self._session
        '''
        Get SQLAlchemy database session.
        
        May only be used for rare bulk operations (as it trades off encapsulation)
        '''
        
        self._max_bind_parameters = 100  # in a sql (sqlite3 limitation)
        
    def commit(self):
        self._session.commit()
        
    def get_gene_by_name(self, name):
        '''
        Get gene by name (including synonyms, ignoring case)
        
        Returns
        -------
        Gene
            Found gene
            
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
    
    


