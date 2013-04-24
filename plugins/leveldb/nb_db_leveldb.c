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

#include "../../nb_plugin_api.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <leveldb/c.h>

struct nb_db_leveldb {
	leveldb_t *instance;
	leveldb_options_t* options;
	leveldb_readoptions_t *roptions;
	leveldb_writeoptions_t *woptions;
};

static struct nb_db *
nb_db_leveldb_open(const struct nb_db_opts *opts)
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

	char* err = NULL;
	leveldb->instance = leveldb_open(leveldb->options, opts->path, &err);
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
nb_db_leveldb_close(struct nb_db *db)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	leveldb_close(leveldb->instance);
	leveldb_readoptions_destroy(leveldb->roptions);
	leveldb_writeoptions_destroy(leveldb->woptions);
	leveldb_options_destroy(leveldb->options);

	leveldb->instance = NULL;

	free(leveldb);
}

static int
nb_db_leveldb_replace(struct nb_db *db, const void *key, size_t key_len,
		      const void *val, size_t val_len)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	char *err = NULL;

	leveldb_put(leveldb->instance, leveldb->woptions, key, key_len,
		    val, val_len, &err);
	if (err != NULL) {
		printf("leveldb_put() failed: %s\n", err);
		return -1;
	}

	return 0;
}

static int
nb_db_leveldb_remove(struct nb_db *db, const void *key, size_t key_len)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	char *err = NULL;

	leveldb_delete(leveldb->instance, leveldb->woptions,
		       key, key_len, &err);
	if (err != NULL) {
		printf("leveldb_delete() failed: %s\n", err);
		return -1;
	}

	return 0;
}

static int
nb_db_leveldb_select(struct nb_db *db, const void *key, size_t key_len,
		     void **pval, size_t *pval_len)
{
	struct nb_db_leveldb *leveldb = (struct nb_db_leveldb *) db;

	char *err = NULL;

	void *val;
	size_t val_len;
	val = leveldb_get(leveldb->instance, leveldb->roptions,
			  key, key_len,
			  &val_len, &err);

	if (val == NULL || err != NULL) {
		printf("leveldb_get() failed: %s\n", err);
		return -1;
	}

	if (pval) {
		*pval = val;
		*pval_len = val_len;
	}

	return 0;
}

static void
nb_db_leveldb_valfree(struct nb_db *db, void *val)
{
	(void) db;
	leveldb_free(val);
}

static struct nb_db_if plugin = {
	.name       = "leveldb",
	.open       = nb_db_leveldb_open,
	.close      = nb_db_leveldb_close,
	.replace    = nb_db_leveldb_replace,
	.remove     = nb_db_leveldb_remove,
	.select     = nb_db_leveldb_select,
	.valfree    = nb_db_leveldb_valfree,
};

NB_DB_PLUGIN const struct nb_db_if *
nb_db_leveldb_plugin(void)
{
	return &plugin;
}
