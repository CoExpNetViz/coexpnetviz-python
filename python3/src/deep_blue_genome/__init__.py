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

from deep_blue_genome.version import __version__

def _init():
    from deep_blue_genome.util import plumbum as pb
    import matplotlib
    from deep_blue_genome.util.database import patch_pymysql
    
    # from Bio import Entrez
    # Entrez.email = 'no-reply@psb.ugent.be'  # TODO perhaps this email address should be user supplied
    
    # init matplotlib
    if not 'DISPLAY' in pb.local.env:
        matplotlib.use('Agg')  # use this backend when no X server
    
    # find __root__
    global __root__
    __root__ = pb.local.path(__file__).dirname
    
    # various
    patch_pymysql()
    
    # setup logging for testing
    # also log everything to stdout
    # XXX logging.basicConfig is easier to set things up
    import sys
    import logging
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    logging.getLogger().root.addHandler(ch)
    logging.getLogger('deep_blue_genome').setLevel(logging.INFO)
    logging.getLogger('deep_blue_genome').setLevel(logging.DEBUG)
    logging.getLogger('deep_blue_genome.core.Database').setLevel(logging.INFO)
    
__root__ = None  # make linter happy
_init()