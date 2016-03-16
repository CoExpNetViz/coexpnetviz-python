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
from sqlalchemy.sql.schema import MetaData

'''
Gene info reading
'''

from plumbum.cmd import wget, gunzip
from plumbum import local, FG
from tempfile import TemporaryDirectory
from deep_blue_genome.core.reader.ncbi import read_gene_info_file_chunked
from chicken_turtle_util.list import remove_duplicates
from chicken_turtle_util.pandas import fill_na_with_none 
from chicken_turtle_util.debug import print_mem
from deep_blue_genome.core.database.gene import Gene, GeneName
from collections import defaultdict
from pandas.core.series import Series
import itertools
import numpy as np

from sqlalchemy import Column, Integer, String, ForeignKey, ForeignKeyConstraint, PrimaryKeyConstraint, Table, func
from sqlalchemy.sql import select, alias, and_

import sys #TODO debug

def load_gene_info(database):
    
    '''
    Load gene info data from NCBI into DBG database
    
    Skips unnamed genes. Deduplicates genes with same name by picking the first
    in file and dropping the rest.
    '''
    
    with TemporaryDirectory() as build_dir:
        build_dir = '.' # TODO rm debug
        with local.cwd(build_dir): # TODO if stuff goes wrong, we redo everything from scratch, seems like we could use a system that prevents redoing stuff... Accessible from python, like, without leaving our application.
#             wget['ftp://ftp.ncbi.nlm.nih.gov/gene/DATA/gene_info.gz']
#             gunzip['gene_info.gz']
            _load_into_database(database)


def _load_into_database(database):
    session = database.session
    
    # TODO Nice to have: I could write something to automatically generate namespaced copies of tables (taking into account foreign keys as well)
    #   gene_name_table = GeneName.__table__.tometadata(meta_data, name='tmp_gene_name')
    #   gene_name_table.c.name.unique = False
    # TODO if using ORM on those classes, you can use the ORM syntax, even though you wouldn't want to do any bulk on them via ORM, ever
    
    # Create temp tables
    meta_data = MetaData(bind=session.bind)
    
    gene_table = Table('tmp_gene', meta_data,
        Column('id', Integer, primary_key=True),
        Column('ncbi_id', Integer, unique=True),
        Column('description', String)
    )
     
    gene_name_table = Table('tmp_gene_name', meta_data,
        Column('id', Integer, primary_key=True),
        Column('name', String(collation='NOCASE'), nullable=False),
        Column('gene_id', Integer, ForeignKey('tmp_gene.id'), nullable=False)
    )
    
    gene_canonical_name_table = Table('tmp_gene_canonical_name', meta_data,
        Column('gene_id', Integer, nullable=False),
        Column('name_id', Integer, nullable=False),
        PrimaryKeyConstraint('gene_id', 'name_id'),
        ForeignKeyConstraint(['gene_id', 'name_id'],['tmp_gene_name.gene_id', 'tmp_gene_name.id'])
    )
    '''1-to-1 association via secondary table to avoid cyclic dependency trouble'''
    
    # TODO write access to Gene, GeneName, and the assoc table should be locked for others
    try:
        meta_data.create_all()

        # Insert into table
        total_unfiltered_rows = 0
        chunk_size = 512 * 2**20 // 14  # x MB / blow up factor
        gene_ids = iter(itertools.count(1)) # TODO start at the MAX id of the current Gene table
        gene_name_ids = iter(itertools.count(1)) #TODO start at...
        for gene_info in read_gene_info_file_chunked('gene_info', chunk_size=chunk_size, usecols=[1,2,4,8]):
            print_mem()
            
            # Ignore unnamed genes
            total_unfiltered_rows += len(gene_info)
            gene_info.dropna(subset=['symbol'], inplace=True)
            
            # Prepare records to insert
            genes = []
            gene_names = []
            gene_canonical_names = []
            fill_na_with_none(gene_info)
            for ncbi_id, name, names, description in gene_info.itertuples():
                # gene
                id = next(gene_ids)
                genes.append({'id': id, 'ncbi_id': int(ncbi_id), 'description': description})  # Note: dataframe loads numbers as np.int64, which isn't supported by sqlalchemy  
                
                # names
                if not names:
                    names = ''
                names = names.split()
                names.append(name)
                names = remove_duplicates(names)
                gene_names.extend({'id': next(gene_name_ids), 'name': n, 'gene_id': id} for n in names)
                
                # canonical name
                gene_canonical_names.append({'gene_id': id, 'name_id': gene_names[-1]['id']})
                    
            print_mem()
            
            # Insert
            session.execute(gene_table.insert(), genes)
            session.execute(gene_name_table.insert(), gene_names)
            session.execute(gene_canonical_name_table.insert(), gene_canonical_names)
            database.commit()
            
            break #TODO debug only
            
        # unnamed genes
        current_total = session.execute(gene_table.count()).scalar()
        unnamed_genes = total_unfiltered_rows - current_total
        
        ###########################################
        # Remove duplicates and add to final tables
        
        # genes
        # TODO should union names2 with GeneName.__table__ in case it already has content
        gene_names1 = alias(gene_name_table)
        gene_names2 = alias(gene_name_table)
        uniqued_genes = gene_table.select().distinct().select_from(
            gene_table.\
            join(gene_names1).\
            outerjoin(gene_names2, and_(
                gene_names1.c.name == gene_names2.c.name,
                gene_names1.c.gene_id > gene_names2.c.gene_id
            ))
        ).where(gene_names2.c.id == None)
        total_duplicates = current_total - session.execute(Gene.__table__.insert().from_select(uniqued_genes.c, uniqued_genes)).rowcount
        print(total_duplicates)
        
        print_mem()
        database.commit()
        print_mem()
        
        # gene names
        uniqued_names = gene_name_table.select().select_from(
            gene_name_table.\
            join(Gene.__table__, Gene.__table__.c.id == gene_name_table.c.gene_id)
        )
        print(session.execute(GeneName.__table__.insert().from_select(uniqued_names.c, uniqued_names)).rowcount)
        
        # gene canonical names
        
        database.commit()
        sys.exit()
#         insert into gene
#         select * from tmp_gene
#         where id in (select distinct(name), gene_id from gene_name order by id)
        
#         result = session.execute(select distinct into real table)
#         total_duplicates = current_total - result.rowcount()  # Actual number of (non-distinct) duplicates in file that were removed
        
    finally:
        # Drop tmp tables
        #meta_data.drop_all() #TODO debug reenable
        pass
    
    # TODO debug rm
    print(database.get_gene_by_name('repa1')) #TODO debug rm
    print(database.get_gene_by_name('repaNope8273983719')) #TODO debug rm
        
    # TODO log anomalies and info

        
                