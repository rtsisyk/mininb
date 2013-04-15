# - Try to find libcascadedb
# Once done this will define
#
#  CASCADEDB_FOUND - system has libcascadedb
#  CASCADEDB_INCLUDE_DIR - the libcascadedb include directory
#  CASCADEDB_LIBRARIES - libcascadedb library
#

find_library(CASCADEDB_LIBRARIES
	NAMES cascadedb
	PATHS "${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES cascadedb
)

find_path(CASCADEDB_INCLUDE_DIRS
	NAMES api.h
	PATHS "${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES cascadedb
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CASCADEDB DEFAULT_MSG CASCADEDB_LIBRARIES CASCADEDB_INCLUDE_DIRS)
mark_as_advanced(CASCADEDB_INCLUDE_DIRS CASCADEDB_LIBRARIES)
