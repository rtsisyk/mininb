project(mininb)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions("-D_POSIX_C_SOURCE=200112L")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -pg")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE
		STRING "Choose the type of build, options are: Debug Release."
		FORCE)
endif()

set(SRC_LIST
	mininb.c
	db.c
	options.c
	random.c
	time.c
	histogram.c
)

find_package (LevelDB QUIET)
if (LEVELDB_FOUND)
	set(SRC_LIST ${SRC_LIST}
		db_leveldb.c
		db_leveldb.h
	)

	include_directories(${LEVELDB_INCLUDE_DIRS})
	set(HAVE_LEVELDB 1)
endif (LEVELDB_FOUND)

find_package (NessDB QUIET)
if (NESSDB_FOUND)
	set(SRC_LIST ${SRC_LIST}
		db_nessdb.c
		db_nessdb.h
	)

	include_directories(${NESSDB_INCLUDE_DIRS})
	if (EXISTS ${NESSDB_INCLUDE_DIRS}/engine/bloom.h)
		message(STATUS "Using NessDB V1 (with LSM)")
		set(HAVE_NESSDB_V1 1)
	else()
		message(STATUS "Using NessDB V2 (with SST)")
		set(HAVE_NESSDB_V2 1)
	endif()
endif (NESSDB_FOUND)

find_package (CascadeDB QUIET)
if (CASCADEDB_FOUND)
	set(SRC_LIST ${SRC_LIST}
		db_cascadedb.c
		db_cascadedb.h
	)

	include_directories(${CASCADEDB_INCLUDE_DIRS})
	set(HAVE_CASCADEDB 1)
endif (CASCADEDB_FOUND)

find_package (BerkeleyDB QUIET)
if (BERKELEYDB_FOUND)
	set(SRC_LIST ${SRC_LIST}
		db_berkeleydb.c
		db_berkeleydb.h
	)

	include_directories(${BERKELEYDB_INCLUDE_DIRS})
	set(HAVE_BERKELEYDB 1)
endif (BERKELEYDB_FOUND)

find_package (KyotoCabinet QUIET)
if (KYOTOCABINET_FOUND)
	set(SRC_LIST ${SRC_LIST}
		db_kyotocabinet.cc
		db_kyotocabinet.h
	)

	include_directories(${KYOTODB_INCLUDE_DIRS})
	set(HAVE_KYOTOCABINET 1)
endif (KYOTOCABINET_FOUND)

configure_file(
	"config.h.cmake"
	"config.h"
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} rt)

if (LEVELDB_FOUND)
	target_link_libraries (${PROJECT_NAME} ${LEVELDB_LIBRARIES})
endif (LEVELDB_FOUND)

if (NESSDB_FOUND)
	target_link_libraries (${PROJECT_NAME} ${NESSDB_LIBRARIES})
endif (NESSDB_FOUND)

if (CASCADEDB_FOUND)
	target_link_libraries (${PROJECT_NAME} ${CASCADEDB_LIBRARIES})
endif (CASCADEDB_FOUND)

if (BERKELEYDB_FOUND)
	target_link_libraries (${PROJECT_NAME} ${BERKELEYDB_LIBRARIES})
endif (BERKELEYDB_FOUND)

if (KYOTOCABINET_FOUND)
	target_link_libraries (${PROJECT_NAME} ${KYOTOCABINET_LIBRARIES})
endif (KYOTOCABINET_FOUND)
