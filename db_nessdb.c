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

#include "db_nessdb.h"

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "config.h"

#include <engine/db.h>

#if !defined(HAVE_NESSDB_V1) && !defined(HAVE_NESSDB_V2)
#error Unsupported NESSDB version
#endif

struct nb_db_nessdb {
	struct nessdb *instance;
};

static struct nb_db *
db_nessdb_ctor(const struct nb_options *opts)
{
	struct nb_db_nessdb *nessdb = malloc(sizeof(*nessdb));
	if (nessdb == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*nessdb));
		goto error_1;
	}

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 1, "%s/%s", opts->root, "nessdb");
	path[FILENAME_MAX - 1] = 0;

	fprintf(stderr, "nessDB new: path=%s\n", path);

#if defined(HAVE_NESSDB_V2)
	nessdb->instance = db_open(path);
#elif defined(HAVE_NESSDB_V1)
	nessdb->instance = db_open(path, 0UL, 0);
#endif
	if (nessdb->instance == NULL) {
		fprintf(stderr, "db_open() failed\n");
		goto error_2;
	}

	return (struct nb_db *) nessdb;

error_2:
	free(nessdb);
error_1:
	return NULL;
}

static void
db_nessdb_dtor(struct nb_db *db)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;
	db_close(nessdb->instance);
	nessdb->instance = NULL;
	free(nessdb);
}

static int
db_nessdb_replace(struct nb_db *db, const struct nb_slice *key,
			    const struct nb_slice *val)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;

	assert (key->size < UINT_MAX);
	assert (val->size < UINT_MAX);

	struct slice nkey, nval;
	nkey.len = key->size;
	nkey.data = (char *) key->data;
	nval.len = val->size;
	nval.data = (char *) val->data;

	/* db_remove(t->instance->db, &nkey); */
	int count = db_add(nessdb->instance, &nkey, &nval);
	if (count != 1) {
		printf("db_add() failed: %d\n", count);
		return -1;
	}

	return 0;
}

static int
db_nessdb_delete(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;

	assert (key->size < UINT_MAX);

	struct slice nkey;
	nkey.len = key->size;
	nkey.data = (char *) key->data;

	db_remove(nessdb->instance, &nkey);

	return 0;
}

static int
db_nessdb_select(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;

	assert (key->size < UINT_MAX);

	struct slice nkey, nval;
	nkey.len = key->size;
	nkey.data = (char *) key->data;

	int count = db_get(nessdb->instance, &nkey, &nval);
	if (count != 1) {
		printf("db_get() failed: %d\n", count);
		return -1;
	}

#if defined(HAVE_NESSDB_SST)
	db_free_data(nval.data);
#endif

	return 0;
}

struct nb_db_if nb_db_nessdb =
{
	.name       = "nessdb",
	.ctor       = db_nessdb_ctor,
	.dtor       = db_nessdb_dtor,
	.replace_cb = db_nessdb_replace,
	.delete_cb  = db_nessdb_delete,
	.select_cb  = db_nessdb_select,
};
