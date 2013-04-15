# - Try to find libkyotocabinet
# Once done this will define
#
#  KYOTOCABINET_FOUND - system has libkyotocabinet
#  KYOTOCABINET_INCLUDE_DIR - the libkyotocabinet include directory
#  KYOTOCABINET_LIBRARIES - libkyotocabinet library
#

find_library(KYOTOCABINET_LIBRARIES
	NAMES kyotocabinet
	PATHS "${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES kyotocabinet
)

find_path(KYOTOCABINET_INCLUDE_DIRS
	NAMES kcpolydb.h
	PATHS "${CMAKE_SOURCE_DIR}/third_party"
	PATH_SUFFIXES kyotocabinet
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KYOTOCABINET DEFAULT_MSG KYOTOCABINET_LIBRARIES KYOTOCABINET_INCLUDE_DIRS)
mark_as_advanced(KYOTOCABINET_INCLUDE_DIRS KYOTOCABINET_LIBRARIES)
