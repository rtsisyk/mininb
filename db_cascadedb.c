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

#include "db_cascadedb.h"

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "config.h"

#include <cascadedb/api.h>

struct nb_db_cascadedb {
	struct db *instance;
};

static struct nb_db *
db_cascadedb_ctor(const struct nb_options *opts)
{
	struct nb_db_cascadedb *cascadedb = malloc(sizeof(*cascadedb));
	if (cascadedb == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*cascadedb));
		goto error_1;
	}

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 1, "%s/%s", opts->root, "cascadedb");
	path[FILENAME_MAX - 1] = 0;

	fprintf(stderr, "cascadeDB new: path=%s\n", path);

	cascadedb->instance = cdb_new(path, NULL);
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
db_cascadedb_dtor(struct nb_db *db)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;
	cdb_delete(cascadedb->instance);
	cascadedb->instance = NULL;
	free(cascadedb);
}

static int
db_cascadedb_replace(struct nb_db *db, const struct nb_slice *key,
		     const struct nb_slice *val)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;

	assert (key->size < UINT_MAX);
	assert (val->size < UINT_MAX);

	struct db_slice  nkey, nval;
	nkey.len = key->size;
	nkey.data = (void *) key->data;
	nval.len = val->size;
	nval.data = (void *) val->data;

	int count = cdb_put(cascadedb->instance, &nkey, &nval,
			    cdb_lsn(cascadedb->instance) + 1);
	if (count != 0) {
		printf("db_put() failed: %d\n", count);
		return -1;
	}

	return 0;
}

static int
db_cascadedb_delete(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;

	assert (key->size < UINT_MAX);

	struct db_slice nkey;
	nkey.len = key->size;
	nkey.data = (char *) key->data;

	cdb_del(cascadedb->instance, &nkey,
		cdb_lsn(cascadedb->instance) + 1);

	return 0;
}

static int
db_cascadedb_select(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_cascadedb *cascadedb = (struct nb_db_cascadedb *) db;

	assert (key->size < UINT_MAX);

	struct db_slice nkey, nval;
	nkey.len = key->size;
	nkey.data = (char *) key->data;

	int count = cdb_get(cascadedb->instance, &nkey, &nval);
	if (count != 1) {
		fprintf(stderr, "db_get() failed: %d\n", count);
		return -1;
	}

	return 0;
}

struct nb_db_if nb_db_cascadedb =
{
	.name       = "cascadedb",
	.ctor       = db_cascadedb_ctor,
	.dtor       = db_cascadedb_dtor,
	.replace_cb = db_cascadedb_replace,
	.delete_cb  = db_cascadedb_delete,
	.select_cb  = db_cascadedb_select,
};
