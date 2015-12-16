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

# TODO there's also a config.py file, merge or remove it

from xdg.BaseDirectory import xdg_config_dirs
from configparser import ConfigParser, ExtendedInterpolation
import plumbum as pb

# TODO perhaps redesign a bit? Like a Configurations(pkg_root, program_name).get(name) -> Configuration.defaults_file or .read() -> actual conf
class Configuration(object):
    
    '''
    Program configuration
    
    Gathers defaults and configuration from package root, /etc and XDG directories.
    ''' 
    
    def __init__(self, pkg_root, program_name, name):
        '''
        Parameters
        ----------
        pkg_root : str
            Package root directory as installed on the system. The defaults file is expected to be found in this directory.
        program_name : str
            Program name, configuration file names are derived from this.
        name : str
            Name of config file to read (without suffix)
        '''
        self._pkg_root = pkg_root
        program_name = program_name.replace(' ', '_').replace('-', '_')
        self._defaults_file = self._pkg_root / '{}.defaults.conf'.format(name)
        self._config_name = '{}.conf'.format(name)
        self._config_dirs = [pb.local.path('/etc')] + [pb.local.path(x) for x in reversed(xdg_config_dirs)]
        self._config_dirs = [pkg_root] + [x / program_name for x in self._config_dirs]
    
    def read(self):
        '''
        Read program configuration as dict-like.
        
        E.g. access the return as `return['section']['key']` -> value.
        
        Values are always strings. Empty strings are allowed.
        '''
        def option_transform(name):
            return name.replace('-', '_').replace(' ', '_').lower()
        
        config_parser = ConfigParser(
            inline_comment_prefixes=('#', ';'), 
            empty_lines_in_values=False, 
            default_section='default', 
            interpolation=ExtendedInterpolation()
        )
        
        config_parser.optionxform = option_transform
        config_parser.read_file(open(self._defaults_file))
        config_files = [x / self._config_name for x in self._config_dirs]
        config_parser.read(config_files)  # reads in given order
        
        return config_parser
    
    @property
    def defaults_file(self):
        '''
        Path to look for config file containing the defaults.
        
        Returns
        -------
        plumbum.Path
        '''
        return self._defaults_file
    
    @property
    def config_dirs(self):
        '''
        Directories to look for config files in.
        
        Returns
        -------
        list of plumbum.Path
        '''
        return self._config_dirs

