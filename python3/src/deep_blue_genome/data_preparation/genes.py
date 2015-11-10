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
from deep_blue_genome.reader.ncbi import read_gene_info_file_chunked

def load_gene_info(database):
    '''
    Load gene info data from NCBI into DBG database
    '''
    with TemporaryDirectory() as build_dir:
        build_dir = '.' # TODO rm debug
        with local.cwd(build_dir): # TODO if stuff goes wrong, we redo everything from scratch, seems like we could use a system that prevents redoing stuff... Accessible from python, like, without leaving our application.
            #wget['ftp://ftp.ncbi.nlm.nih.gov/gene/DATA/gene_info.gz']
            #gunzip['gene_info.gz']
            for gene_info in read_gene_info_file_chunked('gene_info', usecols=[1,2,4,8]):
                print(gene_info.iloc[[0,1,2]])
                #TODO 