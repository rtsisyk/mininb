
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "db_leveldb.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"

#include <leveldb/c.h>

struct nb_db_leveldb {
	leveldb_t *instance;
	leveldb_options_t* options;
	leveldb_readoptions_t *roptions;
	leveldb_writeoptions_t *woptions;
};

static struct nb_db *
db_leveldb_ctor(const struct nb_options *opts)
{
	struct nb_db_leveldb *leveldb = calloc(1, sizeof(*leveldb));
	if (leveldb == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*leveldb));
		goto error_1;
	}

	leveldb->options = leveldb_options_create();
	leveldb->roptions = leveldb_readoptions_create();
	leveldb->woptions = leveldb_writeoptions_create();

	/*
	leveldb_cache_t *cache = leveldb_cache_create_lru(100 * 1024 * 1024);
	leveldb_options_set_cache(leveldb->options, cache);
	*/

	leveldb_writeoptions_set_sync(leveldb->woptions, 0);
	leveldb_readoptions_set_fill_cache(leveldb->roptions, 1);

	leveldb_readoptions_set_verify_checksums(leveldb->roptions, 0);

	leveldb_options_set_compression(leveldb->options,
					leveldb_no_compression);

	leveldb_options_set_info_log(leveldb->options, NULL);
	leveldb_options_set_paranoid_checks(leveldb->options, 0);
	leveldb_options_set_create_if_missing(leveldb->options, 1);

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 1, "%s/%s", opts->root, "leveldb");
	path[FILENAME_MAX - 1] = 0;

	fprintf(stderr, "LevelDB new: path=%s\n", path);

	char* err = NULL;
	leveldb->instance = leveldb_open(leveldb->options, path, &err);
	if (err != NULL) {
		fprintf(stderr, "leveldb_open() failed: %s\n", err);
		leveldb->instance = NULL;
		goto error_2;
	}

	return (struct nb_db *) leveldb;

error_2:
	free(leveldb);
error_1:
	return NULL;
}

static void
db_leveldb_dtor(struct nb_db *db)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	leveldb_close(leveldb->instance);
	leveldb_readoptions_destroy(leveldb->roptions);
	leveldb_writeoptions_destroy(leveldb->woptions);
	leveldb_options_destroy(leveldb->options);

	leveldb->instance = NULL;

	printf("LevelDB delete\n");
}

static int
db_leveldb_replace(struct nb_db *db, const struct nb_slice *key,
		   const struct nb_slice *val)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	char *err = NULL;

	leveldb_put(leveldb->instance, leveldb->woptions, key->data, key->size,
		    val->data, val->size, &err);
	if (err != NULL) {
		printf("leveldb_put() failed: %s\n", err);
		return -1;
	}

	return 0;
}


static int
db_leveldb_delete(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	char *err = NULL;

	leveldb_delete(leveldb->instance, leveldb->woptions,
		       key->data, key->size, &err);
	if (err != NULL) {
		printf("leveldb_delete() failed: %s\n", err);
		return -1;
	}

	return 0;
}


static int
db_leveldb_select(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	char *err = NULL;

	size_t value_size;
	char* value;
	value = leveldb_get(leveldb->instance, leveldb->roptions,
			    key->data, key->size,
			    &value_size, &err);
	if (value) {
		free(value);
	}

	if (err != NULL) {
		printf("leveldb_get() failed: %s\n", err);
		return -1;
	}

	return 0;
}

struct nb_db_if nb_db_leveldb =
{
	.name       = "leveldb",
	.ctor       = db_leveldb_ctor,
	.dtor       = db_leveldb_dtor,
	.replace_cb = db_leveldb_replace,
	.delete_cb  = db_leveldb_delete,
	.select_cb  = db_leveldb_select,
};
