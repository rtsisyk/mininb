set(PLUGIN nb_db_kyotocabinet)
set(PLUGIN_SRC
	${PLUGIN}.cc
)

include_directories(${KYOTOCABINET_INCLUDE_DIRS})

add_library(${PLUGIN} SHARED ${PLUGIN_SRC})
target_link_libraries (${PLUGIN} ${KYOTOCABINET_LIBRARIES})
