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
import click
import deep_blue_genome.core.context as ctx
from deep_blue_genome.core.reader.various import read_expression_matrix_file,\
    read_clustering_file
from deep_blue_genome.core.database.entities import ExpressionMatrix, Clustering
import plumbum as pb
from deep_blue_genome.util.file_system import flatten_paths

'''
The main tool to prepare data for DBG tools
'''

def load_rice_genes(database):
    '''
    Load MSU and RAP gene names
    '''
    
class Context(ctx.CacheMixin, ctx.DatabaseMixin, ctx.TemporaryFilesMixin, ctx.OutputMixin):
    pass


# '''/www/group/biocomp/extra/morph/ARABIDOBSIS/gene_descriptions
# /www/group/biocomp/extra/morph/ITAG/gene_descriptions
# /www/group/biocomp/extra/morph/PGSC/gene_descriptions
# /www/group/biocomp/extra/morph/TOMATO/gene_descriptions
# /www/group/biocomp/extra/morph/rice/annotations
# /www/group/biocomp/extra/morph/rice/msu_to_rap.mapping
# /www/group/biocomp/extra/morph/catharanthus_roseus/functional_annotations'''

clusterings = '''
/www/group/biocomp/extra/morph/ARABIDOBSIS/cluster_solution
/www/group/biocomp/extra/morph/ITAG/cluster_solution
/www/group/biocomp/extra/morph/PGSC/cluster_solution
/www/group/biocomp/extra/morph/TOMATO/cluster_solution
/www/group/biocomp/extra/morph/rice/clusterings
/www/group/biocomp/extra/morph/catharanthus_roseus/clusterings'''.splitlines()[1:]
clusterings = [p.replace('/www/group/biocomp/extra/morph', '/mnt/data/doc/work/prod_data') for p in clusterings]

expression_matrices = '''
/www/group/biocomp/extra/morph/ARABIDOBSIS/data_sets
/www/group/biocomp/extra/morph/ITAG/data_sets
/www/group/biocomp/extra/morph/PGSC/data_sets
/www/group/biocomp/extra/morph/TOMATO/data_sets
/www/group/biocomp/extra/morph/rice/data_sets
/www/group/biocomp/extra/morph/catharanthus_roseus/expression_matrices'''.splitlines()[1:]
expression_matrices = [p.replace('/www/group/biocomp/extra/morph', '/mnt/data/doc/work/prod_data') for p in expression_matrices]

def add_expression_matrix(context, path):
    db = context.database
    with db.scoped_session() as session:
        print('Adding expression matrix: {}'.format(path))
        exp_mat = read_expression_matrix_file(path)  # TODO could speed up by only loading index (=gene names)
        genes = exp_mat.data.index
        genes, missing = db.get_genes_by_name(genes, session)
        exp_mat = ExpressionMatrix(id=db.get_next_id(ExpressionMatrix), path=path, genes=genes)
        # TODO write back without the rows with missing genes
        session.add(exp_mat)
            
def add_clustering(context, path):
    db = context.database
    with db.scoped_session() as session:
        print('Adding clustering: {}'.format(path))
        clustering = read_clustering_file(path)  # TODO could speed up by only loading index (=gene names)
        genes, missing = db.get_genes_by_name(clustering, session)
        clustering = Clustering(id=db.get_next_id(Clustering), path=path, genes=genes)
        # TODO if truly data prep, you'd write back the clustering without the missing genes. You'd probably write it back with the gene ids instead actually, maybe. In that case we probably might as well put it in the database...
        session.add(clustering)
    
@click.command()
@ctx.cli_options(Context) #TODO we still have version on this? Add to cli_options if not
@click.pass_obj
def prepare(main_config, **kwargs):
    '''Create and/or update database.'''
    kwargs['main_config'] = main_config
    context = Context(**kwargs)
    context.database.recreate()
    
    for exp_mat in flatten_paths(map(pb.local.path, expression_matrices)):
        add_expression_matrix(context, exp_mat)
    for clustering in flatten_paths(map(pb.local.path, clusterings)):
        add_clustering(context, clustering)
    
    # TODO a context with... (note that other DBG tools may want some of this contextness too, but not all parts of it; it need be pluggable in code):
    # - output_dir. A no brainer
#     database.recreate()
#     load_gene_info(database)
#     load_rice_genes(database)
#     merge_plaza()
    

# TODO locking? E.g. Should not run morph when database is being rebuilt from scratch. This would happen in a daily or weekly nightly
# batch.
# ... we need to design the required locking (e.g. prep in a separate file, then swap files and in the meantime prevent writes to the previous one or something. Or simply have downtime.)
    
    