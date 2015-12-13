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

# from deep_blue_genome.data_preparation.genes import load_gene_info
import pandas as pd
from deep_blue_genome.core import cli
import click
import deep_blue_genome.core.context as ctx

'''
The main tool to prepare data for DBG tools
'''

def load_rice_genes(database):
    '''
    Load MSU and RAP gene names
    '''
    
class Context(ctx.DatabaseMixin, ctx.CacheMixin, ctx.TemporaryFilesMixin, ctx.OutputMixin):
    pass

@click.command()
@ctx.cli_options(Context)
def prepare(**kwargs):
    '''Create and/or update database.'''
    context = Context(**kwargs)
    # dict_subset('output_dir cache_dir tmp_dir')
    # TODO a context with... (note that other DBG tools may want some of this contextness too, but not all parts of it; it need be pluggable in code):
    # - the database
    # - something for grabbing tmpdirs from. Could be a partial of plumbum with correct root dir
    # - cache dir location? Yes, but not intended to be used directly. We have a downloader or something that uses a subdir of the cache instead.
    # - output_dir. A no brainer
    
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
    
    