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

from deep_blue_genome.core.database.database import Database
# from deep_blue_genome.data_preparation.genes import load_gene_info
import pandas as pd
from deep_blue_genome.core.cli import load_database
from deep_blue_genome.core import cli
import click

'''
The main tool to prepare data for DBG tools
'''

def load_rice_genes(database):
    '''
    Load MSU and RAP gene names
    '''
    
@click.command()
@cli.database_options()
@cli.tmp_dir_option()
@cli.output_dir_option()
@cli.cache_dir_option()
def prepare(**kwargs):
    '''Create and/or update database.'''
    print('prepare')
    print(kwargs)
#     database = load_database(args)
#     database.recreate()
#     load_gene_info(database)
#     load_rice_genes(database)
#     merge_plaza()
#     assert False
    

# TODO We need to commit every now and then to put stuff on disk, but at least
# parts of it should be locked. This would happen in a daily or weekly nightly
# batch.
# ... we need to design the required locking (e.g. prep in a separate file, then swap files and in the meantime prevent writes to the previous one or something. Or simply have downtime.)
    
    