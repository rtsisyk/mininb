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
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include <cascadedb/api.h>

struct nb_db_cascadedb {
	struct db *instance;
};

static struct nb_db *
nb_db_cascadedb_open(const struct nb_db_opts *opts)
{
	struct nb_db_cascadedb *cascadedb = malloc(sizeof(*cascadedb));
	if (cascadedb == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*cascadedb));
		goto error_1;
	}

	cascadedb->instance = cdb_new(opts->path, NULL);
	if (cascadedb->instance == NULL) {
		fprintf(stderr, "db_open() failed\n");
		goto error_2;
	}

	return (struct nb_db *) cascadedb;

error_2:
	free(cascadedb);
error_1:
	return NULL;
}

static void
nb_db_cascadedb_close(struct nb_db *db)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;
	cdb_delete(cascadedb->instance);
	cascadedb->instance = NULL;
	free(cascadedb);
}

static int
nb_db_cascadedb_replace(struct nb_db *db, const void *key, size_t key_len,
			const void *val, size_t val_len)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;

	struct db_slice  nkey, nval;
	nkey.len = key_len;
	nkey.data = (void *) key;
	nval.len = val_len;
	nval.data = (void *) val;

	int count = cdb_put(cascadedb->instance, &nkey, &nval,
			    cdb_lsn(cascadedb->instance) + 1);
	if (count != 0) {
		printf("db_put() failed: %d\n", count);
		return -1;
	}

	return 0;
}

static int
nb_db_cascadedb_remove(struct nb_db *db, const void *key, size_t key_len)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;

	struct db_slice nkey;
	nkey.len = key_len;
	nkey.data = (void *) key;

	cdb_del(cascadedb->instance, &nkey,
		cdb_lsn(cascadedb->instance) + 1);

	return 0;
}

static int
nb_db_cascadedb_select(struct nb_db *db, const void *key, size_t key_len,
		       void **pval, size_t *pval_len)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;

	struct db_slice nkey, nval;
	nkey.len = key_len;
	nkey.data = (char *) key;

	int count = cdb_get(cascadedb->instance, &nkey, &nval);
	if (count != 1) {
		fprintf(stderr, "db_get() failed: %d\n", count);
		return -1;
	}

	if (pval) {
		*pval = nval.data;
		*pval_len = nval.len;
	}

	return 0;
}

static void
nb_db_cascadedb_valfree(struct nb_db *db, void *val)
{
	(void) db;
	free(val);
}

static struct nb_db_if plugin = {
	.name       = "cascadedb",
	.open       = nb_db_cascadedb_open,
	.close      = nb_db_cascadedb_close,
	.replace    = nb_db_cascadedb_replace,
	.remove     = nb_db_cascadedb_remove,
	.select     = nb_db_cascadedb_select,
	.valfree    = nb_db_cascadedb_valfree,
};

NB_DB_PLUGIN const struct nb_db_if *
nb_db_cascadedb_plugin(void)
{
	return &plugin;
}
