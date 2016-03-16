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

'''
MOdule guided Ranking of candidate PatHway genes (MORPH)
'''

from deep_blue_genome.util.plumbum import list_files
from chicken_turtle_util.str import multiline_lstrip
from chicken_turtle_util.various import is_data_file
from chicken_turtle_util import cli
from itertools import repeat
from deep_blue_genome.core.reader.various import read_genes_file
from deep_blue_genome.morph.algorithm import morph as morph_
from deep_blue_genome.core import context as ctx
import click
import pandas as pd
import plumbum as pb
import logging

_logger = logging.getLogger('deep_blue_genome.morph')

class Context(ctx.DatabaseMixin, ctx.OutputMixin, ctx.ConfigurationMixin):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

@click.command()
@cli.argument(
    'baits-file',
    nargs=-1,
    type=click.Path(file_okay=True, dir_okay=True, readable=True, writable=False, exists=True, resolve_path=True),
)
@cli.option(
    '--top-k',
    type=click.IntRange(min=1),
    help='K best candidate genes to output in ranking.'
)
@ctx.cli_options(Context)
@click.pass_obj
def morph(main_config, **kwargs):
    '''
    MOdule guided Ranking of candidate PatHway genes (MORPH).
    
    BAITS_FILE: One or more files or directories containing files listing the bait genes to use.
    '''
    kwargs['main_config'] = main_config # XXX make this DRY
    context = Context(**kwargs)
    
    # Send log to file as well
    logging.getLogger().addHandler(logging.FileHandler(str(context.output_dir / 'morph.log'), mode='w'))

    # Collect options
    top_k = kwargs['top_k']
    
    # Read baits
    def bait_file_to_df(i, path):
        _logger.info('Loading gene group: {}'.format(path))
        series = read_genes_file(path)
        series.index = pd.Index(repeat(i, len(series.index)), name='group_id')
        return series
    bait_file_paths = list(list_files(map(pb.local.path, kwargs['baits_file']), is_data_file))
    bait_file_paths = pd.Series(map(str, bait_file_paths), name='bait_group_file')
    # XXX log each bait file path loaded
    bait_groups = pd.concat(bait_file_to_df(i, path) for i, path in enumerate(bait_file_paths)) #XXX itertuples instead of enumerate
    bait_groups = context.database.get_genes_by_name(bait_groups, map_=True)
    bait_groups.index.name = 'group_id'
    bait_groups = bait_groups.reset_index()
    bait_groups.drop_duplicates(inplace=True)
    
    # Run alg
    rankings = morph_(context, bait_groups, top_k) #XXX in logs, use some kind of name for the bait group, e.g. its path, instead of the less informative group_id

    # Write results to output directory
    ranking_output_dir = context.output_dir / 'rankings'
    ranking_output_dir.mkdir()
    
    rankings = rankings.join(bait_file_paths, on='bait_group_id')
    rankings['output_path'] = rankings.apply(lambda x: ranking_output_dir / '{}.{}.txt'.format(pb.local.path(x.bait_group_file).name, x.bait_group_id), axis=1)
    
    for _, group in rankings.groupby('bait_group_id'):
        best_result = group[group['ausr'] == group['ausr'].max()].iloc[0]
        baits_present = best_result['baits_present']
        baits_missing = best_result['baits_missing']
        ranking = best_result['ranking']
        
        # Convert each Gene to its canonical (or, if none, any other) name
        all_genes = pd.concat((baits_present, baits_missing, ranking.index.to_series()), ignore_index=True)
        all_genes.index = all_genes
        all_genes = all_genes.apply(lambda x: (x.canonical_name or x.names[0]).name) # TODO went so fast, maybe they were loaded eagerly? Not rly, why so fast? Oh well let's just ignore it then? Or... they were added by session just now?
        baits_present = baits_present.map(all_genes)
        baits_missing = baits_missing.map(all_genes)
        baits_present.sort_values(inplace=True)
        baits_missing.sort_values(inplace=True)
        ranking.sort_values(inplace=True, ascending=False)
        ranking.index = ranking.index.to_series().map(all_genes)
        
        # Output result 
        _logger.info('Writing result to {}'.format(best_result.output_path))
        with best_result.output_path.open('w') as f:
            f.write(multiline_lstrip('''
                AUSR: {}
                Bait group file: {}
                Expression matrix used: {}
                Clustering used: {}
                Baits present in both ({}): {}
                Baits missing ({}): {}
                
                Statistics of AUSRs of other rankings of same bait group:
                {}
                
                Candidates:
                {}
                ''').strip().format(
                    best_result['ausr'], 
                    best_result['bait_group_file'],
                    best_result['expression_matrix'].path,
                    best_result['clustering'].path,
                    len(baits_present),
                    ' '.join(baits_present.tolist()),
                    len(baits_missing), 
                    ' '.join(baits_missing.tolist()),
                    group['ausr'].describe().to_string(), # XXX some indent would be nice 
                    ranking.to_string() # XXX some indent (4 spaces)
                )
            )
         
    overview_path = context.output_dir / 'overview.txt'   
    _logger.info('Writing overview of results to {}'.format(overview_path))
    with overview_path.open('w') as f:
        best_ausrs = rankings.groupby('output_path')['ausr'].max()
        best_ausrs.index = best_ausrs.index.to_series().apply(lambda x: x.name)
        best_ausrs.sort_values(inplace=True, ascending=False)
        f.write(multiline_lstrip('''
            Statistics of best AUSRs:
            {}
            
            List of best AUSRs:
            {}
            ''').strip().format(
                best_ausrs.describe().to_string(),
                best_ausrs.to_string(header=False)
            )
        )
    
    # TODO mention missing ones, i.e. those with no generated rankings at all
    
    
    
