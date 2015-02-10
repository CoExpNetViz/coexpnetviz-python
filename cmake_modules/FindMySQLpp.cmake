# Copyright (C)
# 2012 - Paul Weingardt
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
#

IF(MYSQLPP_INCLUDE_DIR)
    SET(MYSQLPP_FIND_QUIETLY TRUE)
ENDIF(MYSQLPP_INCLUDE_DIR)

FIND_PATH(MYSQLPP_INCLUDE_DIR mysql++.h
    /usr/include/mysql++
    /usr/local/include/mysql++)

SET(MYSQLPP_NAMES mysqlpp)
FIND_LIBRARY(MYSQLPP_LIBRARY
    NAMES ${MYSQLPP_NAMES}
    PATHS /usr/lib /usr/local/lib
    PATH_SUFFIXES mysql)

IF(MYSQLPP_INCLUDE_DIR AND MYSQLPP_LIBRARY)
    SET(MYSQLPP_FOUND TRUE)
    SET(MYSQLPP_LIBRARIES ${MYSQLPP_LIBRARY})
ELSE(MYSQLPP_INCLUDE_DIR AND MYSQLPP_LIBRARY)
    SET(MYSQLPP_FOUND FALSE)
ENDIF(MYSQLPP_INCLUDE_DIR AND MYSQLPP_LIBRARY)

IF(MYSQLPP_FOUND)
    IF(NOT MYSQLPP_FIND_QUIETLY)
        MESSAGE(STATUS "Found MySQL++: ${MYSQLPP_LIBRARY}")
    ENDIF(NOT MYSQLPP_FIND_QUIETLY)
ELSE(MYSQLPP_FOUND)
    IF(MYSQLPP_FIND_REQUIRED)
        MESSAGE(STATUS "Looked for MySQL++ libraries named ${MYSQLPP_NAMES}.")
        MESSAGE(FATAL_ERROR "Couldn't find MySQL++ libraries!")
    ELSE(MYSQLPP_FIND_REQUIRED)
        MESSAGE(STATUS "Optional library MySQL++ not found!")
    ENDIF(MYSQLPP_FIND_REQUIRED)
ENDIF(MYSQLPP_FOUND)

MARK_AS_ADVANCED(MYSQLPP_LIBRARY MYSQLPP_INCLUDE_DIR MYSQLPP_FOUND)
