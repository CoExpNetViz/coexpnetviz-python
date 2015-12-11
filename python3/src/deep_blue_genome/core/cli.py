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
'''

import argparse
import pypandoc
from deep_blue_genome.core.database.database import Database

def _create_custom_formatter_class():
    
    '''
    Create a custom formatter class
    
    Returns
    -------
    class
        Custom argparse.HelpFormatter
    '''
    
    class CustomFormatter(argparse.HelpFormatter):
    
        '''
        Custom formatter for argparse help and usage message.
        
        " (default: value)" will be included in the help message of each attribute.
        You can mark attributes as a password attribute so that their default attribute
        is displayed as '*'*8. Extra info about an attribute can be specified using the
        staticmethod `set_attribute_metadata`, e.g. whether it's a password attribute. 
        
        Description and epilog are expected to use markdown syntax. Markdown line
        wrapping will be applied.
        
        HelpFormatter is not public API of argparse, so this may break in future releases.
        But with Python virtual environments we can stick to versions known to work.
        '''
        
        # 'reference' of HelpFormatter: https://hg.python.org/cpython/file/3.4/Lib/argparse.py
        # Some of this code based on Anthon van der Neut's work here: https://bitbucket.org/ruamel/std.argparse/src/cd5e8c944c5793fa9fa16c3af0080ea31f2c6710/__init__.py?at=default&fileviewer=file-view-default
        
        
        ##################
        # Overrides
        
        def add_text(self, text):
            # Custom line wrap, see _format_markdown
            if text is not argparse.SUPPRESS and text is not None:
                self._add_item(self._format_markdown, [text])
            
        def _get_help_string(self, action):
            # Add ' (default: value)' to attr help message 
            help = action.help
            if '%(default)' not in action.help and action.default is not argparse.SUPPRESS:
                defaulting_nargs = [argparse.OPTIONAL, argparse.ZERO_OR_MORE]
                if action.default is not None and (action.option_strings or action.nargs in defaulting_nargs):
                    help += ' (default: %(default)s)'
            return help
    
        def _expand_help(self, action):
            # Mask passwords
            if action.dest in CustomFormatter._password_attributes:
                params = dict(vars(action), prog=self._prog)
                if params.get('default') is not None:
                    params['default'] = '*' * 8
                return self._get_help_string(action) % params
            else:
                return super()._expand_help(action)
        
        
        #################
        # New methods
        
        _password_attributes = set()
        
        def _format_markdown(self, text):
            # Fill text properly (no filling of a list, indented blocks, ...)
            return pypandoc.convert(text, 'md', 'md').replace('\\', '')  # hack: pypandoc happens to wrap lines when converting from markdown to markdown.
        
        @staticmethod
        def set_argument_metadata(action, is_password):
            '''
            action : str
                the variable name corresponding to the attribute as found on the `parse_args` return
            '''
            if is_password:
                CustomFormatter._password_attributes.add(action)
            else:
                CustomFormatter._password_attributes.discard(action)
        
    return CustomFormatter


class ArgumentParser(argparse.ArgumentParser):
    
    '''
    argparse.ArgumentParser combined with defaults from config files.
    
    Also formats help better, though that aspect may break in future versions
    (if breaking changes occur in argparse).
    
    Currently works as a drop in replacement.
    '''
    
    def __init__(self, *args, **kwargs):
        self._CustomFormatter = _create_custom_formatter_class()
        super().__init__(
            *args,
            formatter_class=self._CustomFormatter,
            **kwargs
        )
        
    def add_argument(self, *args, **kwargs):
        '''
        Like `argparse.add_argument`, except...
        
        New parameters
        --------------
        is_password : bool (default=False)
            Whether the attribute expects password values. If so, its default value (if any) will be masked by ``'*'*8``.
        
        Changed parameters
        ------------------
        required : bool
            In addition to regular behaviour, required=True will be ignored if the configuration specifies a default value.
        '''
        # Derive dest (based on https://hg.python.org/cpython/file/3.4/Lib/argparse.py)
        if 'dest' in kwargs:
            dest = kwargs.pop('dest')
        else:
            long_options = [arg for arg in args if arg.startswith('--')]
            short_options = [arg for arg in args if not arg.startswith('--')]  # Short options or positional arg
            if long_options:
                dest = long_options[0]
            else:
                dest = short_options[0]
            dest = dest.lstrip('-').replace('-', '_')
        kwargs['dest'] = dest
        
        # Set required=False if there's a default value
        if any(map(lambda x: self.get_default(dest) is not None, args)):
            kwargs['required'] = False
            
        # Handle other custom kwargs
        self._CustomFormatter.set_argument_metadata(dest, is_password=kwargs.pop('is_password', False))
        
        # Add argument
        arg = super().add_argument(*args, **kwargs)

def load_database(args):
    '''
    Load DBG database
    
    Parameters
    ----------
    args
        Args as returned by `ArgumentParser.parse_args`
    '''
    return Database(args.database_host, args.database_user, args.database_password, args.database_name)


