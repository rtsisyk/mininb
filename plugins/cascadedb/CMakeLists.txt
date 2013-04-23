set(PLUGIN nb_db_cascadedb)
set(PLUGIN_SRC
	${PLUGIN}.c
)

include_directories(${CASCADEDB_INCLUDE_DIRS})

add_library(${PLUGIN} SHARED ${PLUGIN_SRC})
target_link_libraries (${PLUGIN} ${CASCADEDB_LIBRARIES})
