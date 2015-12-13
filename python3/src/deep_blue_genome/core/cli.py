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
click options with better defaults
'''

from functools import partial
import click
from deep_blue_genome import __version__


option = partial(click.option, show_default=True, required=True)
'''Like click.option, but by default show_default=True, required=True'''

def password_option(*args, **kwargs):
    '''
    Like click.option, but by default prompt=True, hide_input=True, show_default=False, required=True.
    '''
    kwargs_ = dict(prompt=True, hide_input=True, show_default=False)
    kwargs_.update(kwargs)
    return option(*args, **kwargs_)

version_option = partial(click.version_option, version=__version__)
'''Version option with DBG version'''

