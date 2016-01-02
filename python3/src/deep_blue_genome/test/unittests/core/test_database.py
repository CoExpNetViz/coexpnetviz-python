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
Test Cache
'''

import pytest
import deep_blue_genome.core.context as ctx
from deep_blue_genome.core import TaskFailedException
from deep_blue_genome.test.util import get_config
import plumbum as pb
from contextlib import contextmanager
    
class Context(ctx.DatabaseMixin):
    pass

non_existing_gene = 'unknown_gene'

@contextmanager
def init(tmp_dir, main_config=None):
    tmp_dir = pb.local.path(tmp_dir)
    config = get_config('prepare', tmp_dir)  # why 'prepare'? No reason, any would have done
    if not main_config:
        main_config = {}
    config['main_config'] = main_config
    context = Context(**config)
    context.database.recreate()  # start from clean database
    
    yield context.database
    
    context.database.dispose()
    

class TestGetGeneByName(object):
    
    def test_get_gene_by_name_add(self, tmpdir):
        with init(tmpdir, dict(exception_handling=dict(unknown_gene='add'))) as database:
            gene = database.get_gene_by_name(non_existing_gene)
            assert gene
            
            gene2 = database.get_gene_by_name(non_existing_gene)
            assert gene == gene2
        
    def test_get_gene_by_name_ignore(self, tmpdir):
        with init(tmpdir, dict(exception_handling=dict(unknown_gene='ignore'))) as database:
            with pytest.raises(GeneNotFoundException):
                database.get_gene_by_name(non_existing_gene)
            
    def test_get_gene_by_name_fail(self, tmpdir):
        with init(tmpdir, dict(exception_handling=dict(unknown_gene='fail'))) as database:
            with pytest.raises(TaskFailedException):
                database.get_gene_by_name(non_existing_gene)
            
    