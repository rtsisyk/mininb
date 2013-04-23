set(PLUGIN nb_db_tokukv)
set(PLUGIN_SRC
	${PLUGIN}.c
)

include_directories(${TOKUKV_INCLUDE_DIRS})

add_library(${PLUGIN} SHARED ${PLUGIN_SRC})
target_link_libraries (${PLUGIN} ${TOKUKV_LIBRARIES})
