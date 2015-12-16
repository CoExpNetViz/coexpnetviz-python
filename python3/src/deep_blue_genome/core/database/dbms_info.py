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

# DBMSInfo = namedtuple('DBMSInfo', 'max_bind_parameters'.split())
# sqlite_innodb_info = DBMSInfo(max_bind_parameters=100)

'''
Limits of particular DBMS
'''

from collections import namedtuple

DBMSInfo = namedtuple('DBMSInfo', 'name version max_index_key_length max_index_key_length_char'.split())

# https://dev.mysql.com/doc/refman/5.5/en/innodb-restrictions.html
mysql_innodb = DBMSInfo(
    name='mysql-innodb', # TODO we use mariadb
    version='5.7.7',
    # max length (inclusive) of a key used in any index
    max_index_key_length = 750, # Until we configure the DB correctly 
    max_index_key_length_char=255,
#     max_index_key_length = 3072,  # in bytes, assuming innodb_large_prefix=ON, which is the default starting from 5.7.7  http://dev.mysql.com/doc/refman/5.7/en/innodb-parameters.html#sysvar_innodb_large_prefix 
#     max_index_key_length_char=3072 // 3, # in utf characters, which is the default charset by default  http://dev.mysql.com/doc/refman/5.7/en/create-index.html and http://stackoverflow.com/a/16474039/1031434 
)

current_db = mysql_innodb
