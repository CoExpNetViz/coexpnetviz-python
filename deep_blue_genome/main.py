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
from deep_blue_genome.morph.main import morph
from chicken_turtle_util import cli
from deep_blue_genome import __root__, __version__

@click.group()
@click.version_option(version=__version__)
def group():
    '''
    Deep Blue Genome (DBG) suite.
    
    Pick a command from the commands listed below. To get help on a command, use `dbg COMMAND --help`.
    '''

# add DBG commands
group.add_command(prepare)
group.add_command(morph)

def load_config():
    cli_config = Configuration(__root__, 'deep_blue_genome', 'cli')
    main_config = Configuration(__root__, 'deep_blue_genome', 'main')
    
    # Format configuration files help section
    config_paths_list = '\n'.join('{}. {!s}'.format(i, path) for i, path in enumerate(cli_config.config_dirs, 1))
    configuration_help = '''
    Configuration files:
    
        The above defaults reflect your current configuration. These defaults can be changed in
        a cli.conf configuration file. The defaults, along with an explanation of the configuration format,
        can be viewed at {}. Deep Blue Genome looks for configuration files in the following directories:
        
        \b
        {}
        
        Any configuration file can override settings of the previous file in the ordering. Some 
        configuration file locations can be changed using the XDG standard (http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html).
        
        Using main.conf, you can configure more advanced options such as how exceptional cases should be handled.
        For the defaults and documentation on the available options, see {}.
    '''.format(cli_config.defaults_file, config_paths_list, main_config.defaults_file)
    configuration_help = '\n'.join(line.lstrip() for line in configuration_help.splitlines()) #TODO click has replacement func?
    
    # Add common help sections to each command
    commands = [group] + list(group.commands.values())
    for command in commands:
        epilog = configuration_help
        if command.epilog:
            epilog += "\n\n" + command.epilog
        command.epilog = epilog
    
    # Read configuration
    cli_config = cli_config.read()
    main_config = main_config.read()
    
    # Convert config to a defaultmap.
    # Merge default section into the section of each sub command
    defaults = {}
    for name in group.commands:
        defaults[name] = dict(cli_config['default'])
        if name in cli_config:
            defaults[name].update(cli_config[name])
        defaults[name] = {k:v for k,v in defaults[name].items() if v} # Note that values are always strings
        
    return defaults, main_config
        
def main(args=None):
    defaults, main_config = load_config()
    
    # Read CLI with defaults applied
    group(args, default_map=defaults, help_option_names=['-h', '--help'], obj=main_config)
