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
from deep_blue_genome.test.util import get_config, get_data_file
from datetime import timedelta, datetime
import plumbum as pb
from freezegun import freeze_time
    
class Context(ctx.CacheMixin):
    pass

def create_context(tmp_dir):
    cache_dir = tmp_dir / 'cache'
    cache_dir.mkdir()
    
    tmp_dir = tmp_dir / 'tmp'
    tmp_dir.mkdir()
    
    config_ = config['prepare'].copy()
    config_.update(
        tmp_dir=tmp_dir,
        cache_dir=cache_dir,
        database_name=config_['database_name'] + '_test',
    )
    
    context = Context(**config_)
    context.database.recreate()  # start from clean database
    return context
    
def test_cache(tmpdir):
    tmpdir = pb.local.path(tmpdir)
    copying_url = 'https://raw.githubusercontent.com/torvalds/linux/9f9499ae8e6415cefc4fe0a96ad0e27864353c89/COPYING'
    copying_local = get_data_file('unittests/cache/COPYING')
    readme_url = 'https://raw.githubusercontent.com/torvalds/linux/9f9499ae8e6415cefc4fe0a96ad0e27864353c89/README'
    readme_local = get_data_file('unittests/cache/README')
    
    # when init cache
    context = create_context(tmpdir)
    cache = context.cache
    cache_dir = tmpdir / 'cache'
    
    # then cache is empty
    assert len(cache_dir.list()) == 0  # starts out empty due to our test setup 
    assert not cache.is_fresh(copying_url)
    assert not cache.is_fresh(readme_url)
    
    # local file is always fresh
    assert cache.is_fresh(copying_local.as_uri())
    
    # when getting local file
    copying_local_cached = cache.get_file(copying_local.as_uri(), expires_after=timedelta(hours=1))
    
    # then it is not added to cache
    assert len(cache_dir.list()) == 0
    assert copying_local_cached == copying_local 
    
    # when get copying file
    cached_copying = cache.get_file(copying_url, expires_after=timedelta(hours=1))
    
    # then copying file is present
    assert len(cache_dir.list()) == 1
    assert_same_file(cached_copying, copying_local)
    assert cache.is_fresh(copying_url)
    assert not cache.is_fresh(readme_url)
    
    # when requesting fresh file
    cache_state = cache_dir.list()
    cached_copying = cache.get_file(copying_url, expires_after=timedelta(hours=1))
    
    # then cache remains unchanged
    assert cache_state == cache_dir.list()
    
    # when requesting other file
    cached_readme = cache.get_file(readme_url, expires_after=timedelta(hours=2))
    
    # then we now have 2 files
    assert len(cache_dir.list()) == 2
    assert_same_file(cached_copying, copying_local)
    assert_same_file(cached_readme, readme_local)
    assert cache.is_fresh(copying_url)
    assert cache.is_fresh(readme_url)
    
    # when copying expires
    with freeze_time(datetime.now() + timedelta(hours=1.1)):
        # then it is no longer fresh
        assert not cache.is_fresh(copying_url)
        assert cache.is_fresh(readme_url)
        assert cache.is_fresh(copying_local.as_uri())  # local files still are always fresh
        
        # when expired file is requested
        cache_state = cache_dir.list()
        cached_copying = cache.get_file(copying_url, expires_after=timedelta(hours=1))
        
        # then it is downloaded anew
        assert cache_state != cache_dir.list()
        
        # and the old version has been removed
        assert len(cache_dir.list()) == 2
    