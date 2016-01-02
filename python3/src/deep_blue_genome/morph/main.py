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

from deep_blue_genome.core.exception_handlers import UnknownGeneHandler
from deep_blue_genome.core.reader.various import read_baits_file
from deep_blue_genome.morph.algorithm import morph as morph_
from deep_blue_genome.core import cli, context as ctx
import click
from deep_blue_genome.util.file_system import flatten_paths
import pandas as pd
import plumbum as pb

class Context(ctx.DatabaseMixin, ctx.OutputMixin):
    pass

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

    # Collect options
    top_k = kwargs['top_k']
    
    # Read baits
    def bait_file_to_df(i, path):
        df = read_baits_file(path).to_frame('bait')
        df['baits_id'] = i
        return df
    bait_file_paths = flatten_paths(map(pb.local.path, kwargs['baits_file']))
    baitss = pd.concat(bait_file_to_df(i, path) for i, path in enumerate(bait_file_paths))
    baitss['bait'] = context.database.get_genes_by_name(baitss[['bait']], map_=True, map_suffix1=True)['bait']
    if context.configuration.unknown_gene_handler == UnknownGeneHandler.ignore:
        baitss.dropna(how='any', inplace=True)    
    
    # Run alg
    ranking = morph_(context, baitss, top_k)

    # Write result to file
    assert False
    ranking.write(context.output_dir)
    
    
    
