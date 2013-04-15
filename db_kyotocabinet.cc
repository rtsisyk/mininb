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

#include "db_kyotocabinet.h"

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"

#include <kcpolydb.h>

struct nb_db_kyotocabinet {
	kyotocabinet::TreeDB instance;
};

static struct nb_db *
db_kyotocabinet_ctor(const struct nb_options *opts)
{
	struct nb_db_kyotocabinet *kyotocabinet =
			new struct nb_db_kyotocabinet();
	assert (kyotocabinet != NULL);

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 4, "%s/%s", opts->root, "kyotocabinet");
	path[FILENAME_MAX - 4] = 0;

	int r;
	r = mkdir(path, 0777);
	if (r != 0 && errno != EEXIST) {
		fprintf(stderr, "mkdir: %d\n", r);
		return NULL;
	}

	strcat(path, "/db");
	fprintf(stderr, "KyotoCabinet new: path=%s\n", path);

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

	return (struct nb_db *) kyotocabinet;

error_2:
	delete kyotocabinet;
	return NULL;
}

static void
db_kyotocabinet_dtor(struct nb_db *db)
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
db_kyotocabinet_replace(struct nb_db *db, const struct nb_slice *key,
		     const struct nb_slice *val)
{
	struct nb_db_kyotocabinet *kc = (struct nb_db_kyotocabinet *) db;

	if (!kc->instance.set((const char *) key->data, key->size,
					(const char *) val->data, val->size)) {
		fprintf(stderr, "db->set() failed\n");
		return -1;
	}

	return 0;
}

static int
db_kyotocabinet_delete(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_kyotocabinet *kc = (struct nb_db_kyotocabinet *) db;

	if (!kc->instance.remove((const char *) key->data, key->size)) {
		fprintf(stderr, "db->remove() failed\n");
		return -1;
	}

	return 0;
}

static int
db_kyotocabinet_select(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_kyotocabinet *kc = (struct nb_db_kyotocabinet *) db;

	if (!kc->instance.get((const char *) key->data, key->size,
			      NULL, 0)) {
		fprintf(stderr, "db->select() failed\n");
		return -1;
	}

	return 0;
}

struct nb_db_if nb_db_kyotocabinet =
{
	.name       = "kyotocabinet",
	.ctor       = db_kyotocabinet_ctor,
	.dtor       = db_kyotocabinet_dtor,
	.replace_cb = db_kyotocabinet_replace,
	.delete_cb  = db_kyotocabinet_delete,
	.select_cb  = db_kyotocabinet_select,
};
