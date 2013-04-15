# - Try to find libleveldb
# Once done this will define
#
#  LEVELDB_FOUND - system has libleveldb
#  LEVELDB_INCLUDE_DIR - the libleveldb include directory
#  LEVELDB_LIBRARIES - libleveldb library
#

find_library(LEVELDB_LIBRARIES NAMES leveldb PATHS ${LEVELDB_LIBRARY_DIRS}
	"${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES leveldb
)
find_path(LEVELDB_INCLUDE_DIRS NAMES leveldb/c.h PATHS
	"${CMAKE_SOURCE_DIR}/third_party"
	"${CMAKE_SOURCE_DIR}/third_party/leveldb/include"
	PATH_SUFFIXES leveldb
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LEVELDB DEFAULT_MSG LEVELDB_LIBRARIES LEVELDB_INCLUDE_DIRS)
mark_as_advanced(LEVELDB_INCLUDE_DIRS LEVELDB_LIBRARIES)

