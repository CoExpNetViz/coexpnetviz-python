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
import click

'''
MOdule guided Ranking of candidate PatHway genes (MORPH)
'''

from deep_blue_genome.core.reader.various import read_baits_file
from deep_blue_genome.morph.algorithm import morph as morph_
from deep_blue_genome.core import cli, context as ctx

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

    # Read files
    top_k = kwargs['top_k']
    for baits_file in kwargs['baits_file']: # TODO support directories, extract some part of `to_paths` to util and reuse it here
        baits = read_baits_file(baits_file)
    
        # Run alg
        print(context, baits, top_k)
        ranking = morph_(context, baits, top_k)

        # Write result to file
        assert False
        ranking.write(context.output_dir)
    
    
    
