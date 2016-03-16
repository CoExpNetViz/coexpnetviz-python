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
Mixins to build a Context class (or 'Application' class if you prefer)

To create a context class: e.g. class MyContext(Mixin1, Mixin2, ...): pass
'''

from deep_blue_genome.core.exception_handlers import UnknownGeneHandler
from collections import namedtuple
from chicken_turtle_util.context import *
from deep_blue_genome.core.cache import Cache
from deep_blue_genome.core.database import Database
import plumbum as pb
import tempfile
    
class ConfigurationMixin(Context):
    
    '''
    Support for a main application configuration.
    
    Expects a dict-like format of an ini file.
    '''
    
    # XXX not reusable across applications due to construction of own config object from app specific conf options, e.g. exception_handlers.unknown_gene
    
    def __init__(self, main_config, **kwargs):
        super().__init__(**kwargs)
        Configuration = namedtuple('Configuration', 'unknown_gene_handler'.split())
        self._config = Configuration(
            unknown_gene_handler=UnknownGeneHandler[main_config['exception_handlers']['unknown_gene']]
        )
        
    @property
    def configuration(self):
        return self._config
        
        
DatabaseMixin = DatabaseMixin(Database)

        
class CacheMixin(DatabaseMixin):
    
    '''
    File cache support.
    
    Also throws DatabaseMixin in the mix.
    '''
    
    _cli_options = [
        cli.option(
            '--cache-dir',
            type=click.Path(file_okay=False, writable=True, exists=True, resolve_path=True),
            help='Directory to place cached data. Cached data is not essential, but may speed up subsequent runs.'
        )
    ]
    
    def __init__(self, cache_dir, **kwargs):
        super().__init__(**kwargs)
        self._cache = Cache(self.database, pb.local.path(cache_dir))
    
    @property
    def cache(self):
        return self._cache
    
    
class TemporaryFilesMixin(Context):
    
    '''
    Temporary file support
    '''
    
    _cli_options = [
        cli.option(
            '--tmp-dir',
            type=click.Path(file_okay=False, writable=True, exists=True, resolve_path=True),
            help='Directory to place temporary files in. Temporary files created by DBG are removed at the end of a run.'
        )
    ]
    
    def __init__(self, tmp_dir, **kwargs):
        super().__init__(**kwargs)
        tempfile.tempdir = str(pb.local.path(tmp_dir))
    
    
class OutputMixin(Context):
    
    '''
    Output file storage support
    '''
    
    _cli_options = [
        cli.option(
            '--output-dir',
            type=click.Path(file_okay=False, writable=True, exists=True, resolve_path=True),
            help='Directory to place the output in.'
        )
    ]
    
    def __init__(self, output_dir, **kwargs):
        super().__init__(**kwargs)
        self._output_dir = pb.local.path(output_dir)
    
    @property
    def output_dir(self):
        return self._output_dir
    