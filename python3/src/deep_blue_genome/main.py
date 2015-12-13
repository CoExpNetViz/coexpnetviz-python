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
Deep Blue Genome entry point
'''

import click
from deep_blue_genome.core.configuration import Configuration
from deep_blue_genome.data_preparation.main import prepare
from deep_blue_genome.core import cli
from deep_blue_genome import __root__

@click.group()
@cli.version_option()
def group():
    '''
    Deep Blue Genome (DBG) suite.
    
    Pick a command from the commands listed below. To get help on a command, use `dbg COMMAND --help`.
    '''

# add DBG commands
group.add_command(prepare)

def main(args):
    config = Configuration(__root__, 'deep_blue_genome')
    
    # Format configuration files help section
    config_paths_list = '\n'.join('{}. {!s}'.format(i, path) for i, path in enumerate(config.config_files, 1))
    configuration_help = '''
    Configuration files:
    
        The above defaults reflect your current configuration. These defaults can be changed in
        configuration files. The defaults, along with an explanation of the configuration format,
        can be viewed at {}. Deep Blue Genome looks for configuration files in the following order:
        
        \b
        {}
        
        Any configuration file can override settings of the previous file in the ordering. Some 
        configuration file locations can be changed using the XDG standard (http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html).
    '''.format(config.defaults_file, config_paths_list)
    configuration_help = '\n'.join(line.lstrip() for line in configuration_help.splitlines()) #TODO click has replacement func?
    
    # Add common help sections to each command
    commands = [group] + list(group.commands.values())
    for command in commands:
        epilog = configuration_help
        if command.epilog:
            epilog += "\n\n" + command.epilog
        command.epilog = epilog
    
    # Read configuration
    config = config.read()
    
    # Convert config to a defaultmap.
    # Merge default section into the section of each sub command
    defaults = {}
    for name in group.commands:
        defaults[name] = dict(config['default'])
        if name in config:
            defaults[name].update(config[name])
        defaults[name] = {k:v for k,v in defaults[name].items() if v} # Note that values are always strings
    
    # Read CLI with defaults applied
    args = [args] if args else []
    group(*args, default_map=defaults, help_option_names=['-h', '--help'])

