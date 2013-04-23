set(PLUGIN nb_db_leveldb)
set(PLUGIN_SRC
	${PLUGIN}.c
)

include_directories(${LEVELDB_INCLUDE_DIRS})

add_library(${PLUGIN} SHARED ${PLUGIN_SRC})
target_link_libraries (${PLUGIN} ${LEVELDB_LIBRARIES})
