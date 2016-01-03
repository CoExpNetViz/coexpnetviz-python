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

import pandas as pd
import click
import deep_blue_genome.core.context as ctx
from deep_blue_genome.core.reader.various import read_expression_matrix_file,\
    read_clustering_file, read_gene_mapping_file
from deep_blue_genome.core.database.entities import ExpressionMatrix, Clustering,\
    GeneMappingTable
import plumbum as pb
from deep_blue_genome.util.file_system import flatten_paths
from deep_blue_genome.util.pandas import df_has_null, series_has_duplicates
import logging
from deep_blue_genome.core.exceptions import TaskFailedException
from deep_blue_genome.util.exceptions import log_exception_msg

_logger = logging.getLogger('deep_blue_genome.prepare')

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
# /www/group/biocomp/extra/morph/catharanthus_roseus/functional_annotations'''

def add_expression_matrix(context, path):
    db = context.database
    with db.scoped_session() as session:
        _logger.info('Adding expression matrix: {}'.format(path))
        # Read file
        exp_mat = read_expression_matrix_file(path)  # XXX could speed up by only loading index (=gene names)
        
        # Get genes from database
        genes = pd.DataFrame(exp_mat.data.index)
        genes = db.get_genes_by_name(genes, session)
        
        # Validate
        if series_has_duplicates(genes):
            raise TaskFailedException('Expression matrix has multiple gene expression rows for some gene')
        
        # Insert in database
        genes = genes['gene'].tolist()
        exp_mat = ExpressionMatrix(id=db.get_next_id(ExpressionMatrix), path=path, genes=genes)
        session.add(exp_mat)
        
        # TODO write back without the rows with missing genes, that's what 'ignore' is about
           
# TODO think about thorough validation for each input here and in file reading and don't forget to make quick todo notes on new input in the future
# TODO the funcs here will be reusable and should be thrown in somewhere else, something in core. We used to call it DataImporter, we won't now
def add_clustering(context, path):
    db = context.database
    with db.scoped_session() as session:
        _logger.info('Adding clustering: {}'.format(path))
        clustering = read_clustering_file(path, name_index=1)  # XXX could speed up by only loading index (=gene names)
        genes = db.get_genes_by_name(clustering[['item']], session)
        clustering = Clustering(id=db.get_next_id(Clustering), path=path, genes=genes['item'].tolist())
        # TODO write back without the rows with missing genes, that's what 'ignore' is about
        # if truly data prep, you'd write back the clustering without the missing genes. You'd probably write it back with the gene ids instead actually, maybe. In that case we probably might as well put it in the database...
        session.add(clustering)
        
def add_gene_mapping(context, path):
    db = context.database
    with db.scoped_session() as session:
        # Read file
        _logger.info('Adding gene mapping from: {}'.format(path))
        mapping = read_gene_mapping_file(path)
        
        # Get genes from database
        mapping = db.get_genes_by_name(mapping, session)
        assert not df_has_null(mapping)  # TODO if unknowngenehandler = ignore, override it in call with fail; instead of asserting
        
        # Insert mappings
        mapping = mapping.applymap(lambda x: x.id)
        mapping.columns = ['left_id', 'right_id']
        mapping.drop_duplicates(inplace=True)
        session.execute(GeneMappingTable.insert(), mapping.to_dict('records'))
    
@click.command()
@ctx.cli_options(Context) #TODO we still have version on this? Add to cli_options if not
@click.pass_obj
def prepare(main_config, **kwargs):
    '''Create and/or update database.'''
    kwargs['main_config'] = main_config
    context = Context(**kwargs)
    context.database.recreate()
    
    def to_paths(listing):
        paths = (p.strip() for p in listing.splitlines())
        paths = [p.replace('/www/group/biocomp/extra/morph', '/mnt/data/doc/work/prod_data') for p in paths if p]
        paths = flatten_paths(map(pb.local.path, paths))
        return paths
    
    gene_mappings = to_paths('''
        /www/group/biocomp/extra/morph/rice/msu_to_rap.mapping
    ''')
    
    expression_matrices = to_paths('''
        /www/group/biocomp/extra/morph/ARABIDOBSIS/data_sets
        /www/group/biocomp/extra/morph/ITAG/data_sets
        /www/group/biocomp/extra/morph/PGSC/data_sets
        /www/group/biocomp/extra/morph/TOMATO/data_sets
        /www/group/biocomp/extra/morph/rice/data_sets
        /www/group/biocomp/extra/morph/catharanthus_roseus/expression_matrices
    ''')
    
    clusterings = to_paths('''
        /www/group/biocomp/extra/morph/ARABIDOBSIS/cluster_solution
        /www/group/biocomp/extra/morph/ITAG/cluster_solution
        /www/group/biocomp/extra/morph/PGSC/cluster_solution
        /www/group/biocomp/extra/morph/TOMATO/cluster_solution
        /www/group/biocomp/extra/morph/rice/clusterings
        /www/group/biocomp/extra/morph/catharanthus_roseus/clusterings
    ''')
    
    for path in gene_mappings:
        with log_exception_msg(_logger, TaskFailedException):
            add_gene_mapping(context, path)
         
    for exp_mat in expression_matrices:
        with log_exception_msg(_logger, TaskFailedException):
            add_expression_matrix(context, exp_mat)
        
    for clustering in clusterings:
        with log_exception_msg(_logger, TaskFailedException):
            add_clustering(context, clustering)
        
    
    # TODO output_dir context
    # TODO dist to output_dir
    
    # XXX old stuff:
#     load_gene_info(database)
#     load_rice_genes(database)
#     merge_plaza()
    

# TODO locking? E.g. Should not run morph when database is being rebuilt from scratch. This would happen in a daily or weekly nightly
# batch.
# ... we need to design the required locking (e.g. prep in a separate file, then swap files and in the meantime prevent writes to the previous one or something. Or simply have downtime.)
    
    