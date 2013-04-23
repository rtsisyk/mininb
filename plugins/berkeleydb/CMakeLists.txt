set(PLUGIN nb_db_berkeleydb)
set(PLUGIN_SRC
	${PLUGIN}.c
)

include_directories(${BERKELEYDB_INCLUDE_DIRS})

add_library(${PLUGIN} SHARED ${PLUGIN_SRC})
target_link_libraries (${PLUGIN} ${BERKELEYDB_LIBRARIES})
