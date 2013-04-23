find_package (LevelDB QUIET)
if (LEVELDB_FOUND)
	add_subdirectory(leveldb)
endif (LEVELDB_FOUND)

find_package (NessDB QUIET)
if (NESSDB_FOUND)
	add_subdirectory(nessdb)
endif (NESSDB_FOUND)

find_package (CascadeDB QUIET)
if (CASCADEDB_FOUND)
	add_subdirectory(cascadedb)
endif (CASCADEDB_FOUND)

find_package (BerkeleyDB QUIET)
if (BERKELEYDB_FOUND)
	add_subdirectory(berkeleydb)
endif (BERKELEYDB_FOUND)

find_package (KyotoCabinet QUIET)
if (KYOTOCABINET_FOUND)
	add_subdirectory(kyotocabinet)
endif (KYOTOCABINET_FOUND)

find_package (TokuKV QUIET)
if (TOKUKV_FOUND)
	add_subdirectory(tokukv)
endif (TOKUKV_FOUND)
