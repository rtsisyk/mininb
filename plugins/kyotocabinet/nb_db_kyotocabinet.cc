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

#include <sys/stat.h>
#include <sys/types.h>

#include <kcpolydb.h>

struct nb_db_kyotocabinet {
	struct nb_db base;
	kyotocabinet::TreeDB instance;
};

static struct nb_db *
nb_db_kyotocabinet_open(const struct nb_db_opts *opts)
{
	struct nb_db_kyotocabinet *kyotocabinet =
			new struct nb_db_kyotocabinet();
	assert (kyotocabinet != NULL);


	int r;
	r = mkdir(opts->path, 0777);
	if (r != 0 && errno != EEXIST) {
		fprintf(stderr, "mkdir: %d\n", r);
		return NULL;
	}

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 4, "%s/db", opts->path);
	path[FILENAME_MAX - 1] = 0;

	int open_options = kyotocabinet::PolyDB::OWRITER |
			   kyotocabinet::PolyDB::OCREATE;
	int tune_options = kyotocabinet::TreeDB::TSMALL |
			 kyotocabinet::TreeDB::TLINEAR;
	kyotocabinet->instance.tune_options(tune_options);
	//kyotocabinet->instance.tune_page(1024);

	if (!kyotocabinet->instance.open(path, open_options)) {
		fprintf(stderr, "db->open failed: %s\n",
			kyotocabinet->instance.error().name());
		goto error_2;
	}

	kyotocabinet->base.opts = opts;
	return &kyotocabinet->base;

error_2:
	delete kyotocabinet;
	return NULL;
}

static void
nb_db_kyotocabinet_close(struct nb_db *db)
{
	struct nb_db_kyotocabinet *kyotocabinet =
			(struct nb_db_kyotocabinet *) db;

	if (!kyotocabinet->instance.close()) {
		fprintf(stderr, "db->close failed: %s\n",
			kyotocabinet->instance.error().name());
	}

	delete kyotocabinet;
}

static int
nb_db_kyotocabinet_replace(struct nb_db *db, const void *key, size_t key_len,
		      const void *val, size_t val_len)
{
	struct nb_db_kyotocabinet *kc = (struct nb_db_kyotocabinet *) db;

	if (!kc->instance.set((const char *) key, key_len,
			      (const char *) val, val_len)) {
		fprintf(stderr, "db->set() failed\n");
		return -1;
	}

	return 0;
}

static int
nb_db_kyotocabinet_remove(struct nb_db *db, const void *key, size_t key_len)
{
	struct nb_db_kyotocabinet *kc = (struct nb_db_kyotocabinet *) db;

	if (!kc->instance.remove((const char *) key, key_len)) {
		fprintf(stderr, "db->remove() failed\n");
		return -1;
	}

	return 0;
}

static int
nb_db_kyotocabinet_select(struct nb_db *db, const void *key, size_t key_len,
		     void **pval, size_t *pval_len)
{
	struct nb_db_kyotocabinet *kc = (struct nb_db_kyotocabinet *) db;

	assert (pval == NULL);

	if (!kc->instance.get((const char *) key, key_len,
			      NULL, 0)) {
		fprintf(stderr, "db->select() failed\n");
		return -1;
	}

	return 0;
}

static void
nb_db_kyotocabinet_valfree(struct nb_db *db, void *val)
{
	(void) db;
	free(val);
}

static struct nb_db_if plugin = {
	.name       = "kyotocabinet",
	.open       = nb_db_kyotocabinet_open,
	.close      = nb_db_kyotocabinet_close,
	.replace    = nb_db_kyotocabinet_replace,
	.remove     = nb_db_kyotocabinet_remove,
	.select     = nb_db_kyotocabinet_select,
	.valfree    = nb_db_kyotocabinet_valfree,
};

extern "C" NB_DB_PLUGIN const struct nb_db_if *
nb_db_kyotocabinet_plugin(void)
{
	return &plugin;
}
