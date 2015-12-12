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

'''
CLI support classes

Provides:

- more options for click
- options specific to DBG
'''

from functools import partial
import click
from deep_blue_genome import __version__
from deep_blue_genome.core.util import compose, dict_subset
from deep_blue_genome.core.database.database import Database


option = partial(click.option, show_default=True, required=True)
'''Like click.option, but by default show_default=True, required=True'''

def password_option(*args, **kwargs):
    '''
    Like click.option, but by default prompt=True, hide_input=True, show_default=False, required=True.
    '''
    kwargs_ = dict(prompt=True, hide_input=True, show_default=False)
    kwargs_.update(kwargs)
    return option(*args, **kwargs_)


tmp_dir_option = partial(option, '--tmp-dir', help='Directory to place temporary files in. Temporary files created by DBG are removed at the end of a run.')
output_dir_option = partial(option, '--output-dir', help='Directory to place the output in.')
cache_dir_option = partial(option, '--cache-dir', help='Directory to place cached data. Cached data is not essential, but may speed up subsequent runs.')

def database_options():
    '''
    Database configuration options
    '''
    return compose(
        option('--database-host', help='Host running the database to connect to. Provide its DNS or IP.'),
        option('--database-user', help='User name to authenticate with.'),
        password_option('--database-password', help='Password corresponding to user to authenticate with.'),
        option('--database-name', help='Name to use for SQL database on given host.'),
    )

def load_database(kwargs):
    '''
    Load database from CLI kwargs
    
    Parameters
    ----------
    kwargs
        Keyword args as given by click to a command
    '''
    return Database(**dict_subset('database_host database_user database_password database_name'.split()))


version_option = partial(click.version_option, version=__version__)
'''Version option with DBG version'''

