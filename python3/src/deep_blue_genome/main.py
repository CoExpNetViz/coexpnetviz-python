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

import sys
from deep_blue_genome.core.cli import ArgumentParser
from functools import partial
from xdg.BaseDirectory import xdg_config_dirs
from deep_blue_genome import __root__
from configparser import ConfigParser, ExtendedInterpolation
import plumbum as pb

'''
Deep Blue Genome entry point
'''

def _load_config(name):
    '''
    name : str
        Configuration file name (without file extensions). E.g. 'shiny_new_project'.
    '''
    # config files
    default_config_file = __root__ / '{}.defaults.conf'.format(name)
    config_name = '{}.conf'.format(name)
    user_config_paths = [__root__ / config_name, pb.local.path('/etc') / config_name] + [pb.local.path(x) / config_name for x in reversed(xdg_config_dirs)]
    
    # read configs
    def option_transform(name):
        return name.replace('-', '_').replace(' ', '_').lower()
    config_parser = ConfigParser(
        inline_comment_prefixes=('#', ';'), 
        empty_lines_in_values=False, 
        default_section='default', 
        interpolation=ExtendedInterpolation()
    )
    config_parser.optionxform = option_transform
    config_parser.read_file(open(default_config_file))
    config_parser.read(user_config_paths)  # read in given order
    
    # CLI configuration files help
    config_paths_list = '\n'.join('{}. {!s}'.format(i, path) for i, path in enumerate(user_config_paths, 1))
    config_files_help = '''
        The above defaults reflect your current configuration. These defaults can be changed in
        configuration files. The defaults, along with an explanation of the configuration format,
        can be viewed at {}. Deep Blue Genome looks for configuration files in the following order:
        
        {}
        
        Any configuration file can override settings of the previous file in the ordering. Some 
        configuration file locations can be changed using the XDG standard (http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html).
    '''.format(default_config_file, config_paths_list)
    config_files_help = '\n'.join(line.lstrip() for line in config_files_help.splitlines())
    
    return config_parser, config_files_help

def _set_defaults(config, parser, section):
    '''
    Set parser defaults using config
    '''
    config = config[section] if section in config else config['default']
    config = {k:v for k,v in config.items() if v != ''}
    parser.set_defaults(**config)
    
def _RootParser(config, config_files_help):
    parser = ArgumentParser('dbg', description='Deep Blue Genome (DBG).')
    parser.add_argument(
        '--database-host', required=True,
        help='Host running the database to connect to. Provide its DNS or IP.'
    )
    parser.add_argument(
        '--database-user', required=True,
        help='User name to authenticate with.'
    )
    parser.add_argument(
        '--database-password', required=True, is_password=True,
        help='Password corresponding to user to authenticate with.'
    )
    parser.add_argument(
        '--database-name', required=True,
        help='Name to use for SQL database on given host.'
    )
    parser.add_argument(
        '--tmp-dir', required=True,
        help='Directory to place temporary directories in. This directory can safely be cleared after the run.'
    )
    parser.add_argument( # TODO not all make use of the output-dir yet
        '--output-dir', required=True,
        help='Directory to place output in, if any.'
    )
    _set_defaults(config, parser, 'default')
    
    config_files_section = parser.add_argument_group('Configuration files', description=config_files_help)
    
    return parser

def _SubParser(subparsers, config, name, description, cache_required=False):
    epilog = 'For general arguments and general information, e.g. details on configuration files, see `dbg -h`'
    parser = subparsers.add_parser(name, description=description, epilog=epilog)
    if cache_required:
        parser.add_argument(
            '--cache-dir', required=True,
            help='Directory to place cached data. This directory can safely be cleared after the run, but subsequent runs could be faster if you do not'
        )
    _set_defaults(config, parser, name.replace('-', '_'))
    return parser
    
def _DataPreparationSubParser(SubParser):
    SubParser('prepare', 'Create and or update database.', cache_required=True)
    
def main(argv=None):
    if not argv:
        argv = sys.argv
        
    config, config_files_help = _load_config('deep_blue_genome')
    
    parser = _RootParser(config, config_files_help)
    subparsers = parser.add_subparsers(title='Sub commands',
        help='E.g. to invoke the sub command `morph`, use `dbg morph`. To get help on a sub command use the -h option, e.g. `dbg morph -h`.')
    SubParser = partial(_SubParser, subparsers, config)
#     _MORPHSubParser(SubParser)
#     _CoExpNetVizSubParser(SubParser)
    _DataPreparationSubParser(SubParser)
    
    args = parser.parse_args(argv[1:])
    assert False
    