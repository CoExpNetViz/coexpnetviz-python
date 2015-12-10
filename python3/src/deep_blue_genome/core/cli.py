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

import configargparse
from xdg.BaseDirectory import xdg_config_dirs
from deep_blue_genome import __root__
from deep_blue_genome.core.database.database import Database

def ArgumentParser(**kwargs):
    config_name = 'deep_blue_genome.conf'
    config_file_paths = [__root__ / config_name, '/etc/' + config_name] + [x + "/" + config_name for x in reversed(xdg_config_dirs)]  # default config locations
    parser = configargparse.ArgumentParser(default_config_files=config_file_paths, **kwargs)
    parser.add_argument(
        '--database-host', required=True,
        help='Host running the database to connect to. Provide its DNS or IP.'
    )
    parser.add_argument(
        '--database-user', required=True,
        help='User name to authenticate with.'
    )
    parser.add_argument(
        '--database-password', required=True,
        help='Password corresponding to user to authenticate with.'
    )
    parser.add_argument(
        '--database-name', required=True,
        help='Name to use for SQL database on given host.'
    )
    return parser

def load_database(args):
    '''
    Load DBG database
    
    Parameters
    ----------
    args
        Args as returned by `ArgumentParser.parse_args`
    '''
    return Database(args.database_host, args.database_user, args.database_password, args.database_name)
