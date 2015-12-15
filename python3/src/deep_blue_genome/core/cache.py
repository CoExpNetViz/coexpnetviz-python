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


from urllib.parse import urlparse
from deep_blue_genome.core.util import download_file
from deep_blue_genome.core.database.entities import CachedFile
from datetime import datetime
import plumbum as pb

class Cache(object):
    
    '''
    Local cache of remote files
    
    Supports multiple instances across threads and processes to the same
    `cache_dir` as long as they all use the same database.
    
    Cache integrity is maintained across crashes, though some unused data may
    become present in the cache.
    '''
    
    def __init__(self, database, cache_dir):
        '''
        Parameters
        ----------
        database : Database
        cache_dir : plumbum.Path
            Root directory of the cache, i.e. directory in which to put cached data.
        '''
        self._database = database
        self._session = database.create_session()
        self._root_dir = cache_dir
        
    def is_fresh(self, url):
        '''
        Get whether file is fresh.
        
        Local files are always considered fresh (i.e. if url has the 'file'
        scheme). Other files are only considered fresh if the file is present in
        the cache and not yet expired.
        '''
        url_parts = urlparse(url)
        if url_parts.scheme == 'file':
            return True
        else:
            record = self._get_cached_file(url)
            if record:
                return not record.expired
            else:
                return False
        
    def get_file(self, url, expires_after):
        '''
        Get file from cache or retrieve it from `uri` if expired.
        
        Parameters
        ----------
        url : str
            The file to get from cache, identified by the URL where it can be
            retrieved from. (Although admittedly, multiple URLs can point to the
            same object). If url is of the 'file' scheme, the file will not be
            copied to the cache and its path is returned.
        expires_after : timedelta
            A time delta after which the file will expire, i.e. expiry is set to
            `now + expires_after`. This is only applied if the file was
            expired/missing in the cache. E.g. a fresh file that will expire in
            an hour, will not have its expiry postponed.
        
        Returns
        -------
        plumbum.Path
            Path to (read-only) file in cache
        '''        
        # If file://path, return path 
        url_parts = urlparse(url)
        if url_parts.scheme == 'file':
            return url_parts.path
        
        # Else, check cache
        record = self._get_cached_file(url)
            
        if not record or record.expired:
            record = self._refresh_file(record, url, expires_after)
        
        return pb.local.path(record.path)
        
    def _refresh_file(self, record, url, expires_after):
        '''
        Get file from scratch (i.e. not from cache), and add to cache
        '''
        
        # Make sure we have a record and find the next_dir to download to
        if record:
            old_dir = pb.local.path(record.path).dirname
            next_version = int(old_dir.suffix[1:]) + 1
            next_dir = old_dir.with_suffix('.{}'.format(next_version))
        else:
            old_dir = None
            record = CachedFile(id=self._database.get_next_id(CachedFile), source_url=url)
            next_dir = self._root_dir / '{}.0'.format(record.id)
            self._session.add(record)
        
        # Download and update record on success
        try:
            next_dir.mkdir()
            record.path = str(download_file(url, next_dir))
            record.cached_at = datetime.now()
            record.expires_at = datetime.now() + expires_after
            self._session.commit()
        except:
            next_dir.delete()
            raise
        
        # Get rid of old version
        if old_dir:
            old_dir.delete()
            
        return record
    
    def _get_cached_file(self, source_url):
        return self._session.query(CachedFile).filter_by(source_url = source_url).first()
        