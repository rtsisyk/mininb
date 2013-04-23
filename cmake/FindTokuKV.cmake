# - Try to find libtokudb
# Once done this will define
#
#  TOKUKV_FOUND - system has libdb
#  TOKUKV_INCLUDE_DIR - the libdb include directory
#  TOKUKV_LIBRARIES - libdb library
#

find_library(TOKUKV_LIBRARIES
	NAMES tokudb
	PATHS "${CMAKE_SOURCE_DIR}/third_party/ft-index/out/lib"
)

find_path(TOKUKV_INCLUDE_DIRS
	NAMES tokudb.h
	PATHS "${CMAKE_SOURCE_DIR}/third_party/ft-index/out/include"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TOKUKV DEFAULT_MSG TOKUKV_LIBRARIES TOKUKV_INCLUDE_DIRS)
mark_as_advanced(TOKUKV_INCLUDE_DIRS TOKUKV_LIBRARIES)
