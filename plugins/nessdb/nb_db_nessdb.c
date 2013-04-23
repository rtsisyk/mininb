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
nb_db_nessdb_open(const struct nb_db_opts *opts)
{
	struct nb_db_nessdb *nessdb = malloc(sizeof(*nessdb));
	if (nessdb == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*nessdb));
		goto error_1;
	}

#if defined(HAVE_NESSDB_V2)
	nessdb->instance = db_open(opts->path);
#elif defined(HAVE_NESSDB_V1)
	nessdb->instance = db_open(opts->path, 0UL, 0);
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
nb_db_nessdb_close(struct nb_db *db)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;
	db_close(nessdb->instance);
	nessdb->instance = NULL;
	free(nessdb);
}

static int
nb_db_nessdb_replace(struct nb_db *db, const void *key, size_t key_len,
		     const void *val, size_t val_len)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;

	assert (key_len < UINT_MAX);
	assert (val_len < UINT_MAX);

	struct slice nkey, nval;
	nkey.len = key_len;
	nkey.data = (char *) key;
	nval.len = val_len;
	nval.data = (char *) val;

	int count = db_add(nessdb->instance, &nkey, &nval);
	if (count != 1) {
		printf("db_add() failed: %d\n", count);
		return -1;
	}

	return 0;
}

static int
nb_db_nessdb_remove(struct nb_db *db, const void *key, size_t key_len)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;

	assert (key_len < UINT_MAX);

	struct slice nkey;
	nkey.len = key_len;
	nkey.data = (char *) key;

	db_remove(nessdb->instance, &nkey);

	return 0;
}

static int
nb_db_nessdb_select(struct nb_db *db, const void *key, size_t key_len,
		     void **pval, size_t *pval_len)
{
	struct nb_db_nessdb *nessdb = (struct nb_db_nessdb *) db;

	assert (key_len < UINT_MAX);

	struct slice nkey, nval;
	nkey.len = key_len;
	nkey.data = (char *) key;

	int count = db_get(nessdb->instance, &nkey, &nval);
	if (count != 1) {
		printf("db_get() failed: %d\n", count);
		return -1;
	}

	if (pval) {
		*pval = nval.data;
		*pval_len = nval.len;
	} else {
#if defined(HAVE_NESSDB_V2)
		db_free_data(nval.data);
#endif
	}

	return 0;
}

static void
nb_db_nessdb_valfree(struct nb_db *db, void *val)
{
	(void) db;
	(void) val;
#if defined(HAVE_NESSDB_V2)
	db_free_data(val);
#endif
}

static struct nb_db_if plugin = {
	.name       = "nessdb",
	.open       = nb_db_nessdb_open,
	.close      = nb_db_nessdb_close,
	.replace    = nb_db_nessdb_replace,
	.remove     = nb_db_nessdb_remove,
	.select     = nb_db_nessdb_select,
	.valfree    = nb_db_nessdb_valfree,
};

NB_DB_PLUGIN const struct nb_db_if *
nb_db_nessdb_plugin(void)
{
	return &plugin;
}

