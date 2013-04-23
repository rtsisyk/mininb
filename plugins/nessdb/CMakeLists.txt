set(PLUGIN nb_db_nessdb)
set(PLUGIN_SRC
	${PLUGIN}.c
)

include_directories(${NESSDB_INCLUDE_DIRS})
if (EXISTS ${NESSDB_INCLUDE_DIRS}/engine/bloom.h)
	message(STATUS "Using NessDB V1 (with LSM)")
	set(HAVE_NESSDB_V1 1)
else()
	message(STATUS "Using NessDB V2 (with SST)")
	set(HAVE_NESSDB_V2 1)
endif()

configure_file(
	"config.h.cmake"
	"config.h"
)


add_library(${PLUGIN} SHARED ${PLUGIN_SRC})
target_link_libraries (${PLUGIN} ${NESSDB_LIBRARIES})
