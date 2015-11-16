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

from plumbum.cmd import wget, gunzip
from plumbum import local, FG
from tempfile import TemporaryDirectory
from deep_blue_genome.core.reader.ncbi import read_gene_info_file_chunked
from deep_blue_genome.util import fill_na_with_none, print_mem
from deep_blue_genome.core.database.gene import Gene, GeneName
from collections import defaultdict
from pandas.core.series import Series
from memory_profiler import profile

def load_gene_info(database):
    
    '''
    Load gene info data from NCBI into DBG database
    
    Skips unnamed genes.
    '''

    def as_gene(row):
        '''
        Gene info row tuple to database.add_genes namedtuple
        '''
        names = [row['symbol']]
        if row['synonyms']:
            names.extend(row['synonyms'])
        names = list(map(lambda x: GeneName(name=x), names))
        return Gene(ncbi_id=row.name, name=names[0], names=names, description=row['description'])
    
    with TemporaryDirectory() as build_dir:
        build_dir = '.' # TODO rm debug
        with local.cwd(build_dir): # TODO if stuff goes wrong, we redo everything from scratch, seems like we could use a system that prevents redoing stuff... Accessible from python, like, without leaving our application.
#             wget['ftp://ftp.ncbi.nlm.nih.gov/gene/DATA/gene_info.gz']
#             gunzip['gene_info.gz']
            total_rows = 0
            unnamed_genes = 0
            total_duplicates = 0  # lower bound of number of duplicates
            chunk_size = 256 * 2**20 / 45  # x MB / blow up factor
            for gene_info in read_gene_info_file_chunked('gene_info', chunk_size=chunk_size, usecols=[1,2,4,8]):
                print_mem()
                import sys #TODO debug
                
                # Ignore unnamed genes
                original_count = len(gene_info)
                total_rows += original_count
                gene_info.dropna(subset=['symbol'], inplace=True)
                unnamed_genes += original_count - len(gene_info)
                print_mem()
                
                # Ignore duplicates (name duplicates, not biological gene duplicates)
                original_count = len(gene_info)
                gene_info.drop_duplicates(subset='symbol', inplace=True)
                total_duplicates += original_count - len(gene_info)
                print_mem()
                
                # Add to database
                fill_na_with_none(gene_info)
                genes = [as_gene(row) for i, row in gene_info.iterrows()]
                print_mem()
                database.add_genes(genes)
                database.commit()
                
                sys.exit()
                
        # TODO log anomalies etc
                
                