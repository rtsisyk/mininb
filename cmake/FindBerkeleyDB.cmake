# - Try to find libberkeleydb
# Once done this will define
#
#  BERKELEYDB_FOUND - system has libdb
#  BERKELEYDB_INCLUDE_DIR - the libdb include directory
#  BERKELEYDB_LIBRARIES - libdb library
#

find_library(BERKELEYDB_LIBRARIES
	NAMES db-48 db-4.8 db-51 db-5.1
	PATHS "${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES db db-48 db-4.8 db-51 db-5.1
)

find_path(BERKELEYDB_INCLUDE_DIRS
	NAMES db.h
	PATHS "${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES db db-48 db-4.8 db-51 db-5.1
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BERKELEYDB DEFAULT_MSG BERKELEYDB_LIBRARIES BERKELEYDB_INCLUDE_DIRS)
mark_as_advanced(BERKELEYDB_INCLUDE_DIRS BERKELEYDB_LIBRARIES)
