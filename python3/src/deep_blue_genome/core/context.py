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
from deep_blue_genome.core.exception_handlers import UnknownGeneHandler

'''
Mixins to build a Context class (or 'Application' class if you prefer)

To create a context class: e.g. class MyContext(Context, Mixin1, Mixin2, ...): pass
'''

from deep_blue_genome.core.util import compose, flatten
from deep_blue_genome.core.database.database import Database
from deep_blue_genome.core import cli
from deep_blue_genome.core.cache import Cache
import click
import plumbum as pb
import tempfile
        
def cli_options(class_):
    '''
    Get click CLI options of the built context class 
    '''
    options = flatten([cls._cli_options for cls in class_.__mro__[1:] if hasattr(cls, '_cli_options')])
    return compose(*options)


class ConfigurationMixin(object):
    
    '''
    Support for a main application configuration.
    
    Expects a dict-like format of an ini file
    '''
    
    def __init__(self, main_config, **kwargs):
        self._config = {k : dict(v) for k,v in main_config.items()}
        self._config['exception_handlers']['unknown_gene'] = UnknownGeneHandler[self._config['exception_handlers']['unknown_gene']]
        
    @property
    def configuration(self):
        return self._config
        
        
class DatabaseMixin(ConfigurationMixin):
    
    '''
    Database access
    '''
    
    _cli_options = [
        cli.option('--database-host', help='Host running the database to connect to. Provide its DNS or IP.'),
        cli.option('--database-user', help='User name to authenticate with.'),
        cli.password_option('--database-password', help='Password corresponding to user to authenticate with.'),
        cli.option('--database-name', help='Name to use for SQL database on given host.'),
    ]
    
    def __init__(self, database_host, database_user, database_password, database_name, **kwargs):
        super().__init__(**kwargs)
        self._database = Database(self, host=database_host, user=database_user, password=database_password, name=database_name)
    
    @property
    def database(self):
        return self._database
        
        
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
    
    
class TemporaryFilesMixin(object):
    
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
        tempfile.tempdir = str(pb.local.path(tmp_dir))
    
    
class OutputMixin(object):
    
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
        self._output_dir = pb.local.path(output_dir)
    
    @property
    def output_dir(self):
        return self._output_dir
    