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

#include "db_tokukv.h"

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"

#include <tokudb.h>

struct nb_db_tokukv {
	DB_ENV *env;
	DB *db;
};

static struct nb_db *
db_tokukv_ctor(const struct nb_options *opts)
{
	struct nb_db_tokukv *tokukv = malloc(sizeof(*tokukv));
	if (tokukv == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*tokukv));
		goto error_1;
	}

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 1, "%s/%s", opts->root, "tokukv");
	path[FILENAME_MAX - 1] = 0;

	fprintf(stderr, "TokuKV new: path=%s\n", path);

	int r;
	r = mkdir(path, 0777);
	if (r != 0 && errno != EEXIST) {
		fprintf(stderr, "mkdir: %d\n", r);
		goto error_1;
	}

	DB_ENV *env;
	DB *db;

	int env_flags = 0;
	r = db_env_create(&env, env_flags);
	if (r != 0) {
		fprintf(stderr, "db_env_create failed: %s\n",
			db_strerror(r));
		goto error_1;
	}

	if (env->set_cachesize) {
		r = env->set_cachesize(env, 0, 127*1024*1024, 1);
		if (r != 0) {
			fprintf(stderr, "env->set_cachesize failed: %s\n",
				db_strerror(r));
			goto error_2;
		}
	}

#if 0
	env_flags |= DB_TXN_WRITE_NOSYNC;
	r = env->set_flags(env, env_flags, 1);
	if (r != 0) {
		fprintf(stderr, "set_flags: %d\n", r);
		goto error_2;
	}
#endif

	int env_open_flags = DB_CREATE|DB_PRIVATE|DB_INIT_MPOOL;
	r = env->open(env, path, env_open_flags, 0644);
	if (r != 0) {
		fprintf(stderr, "env->open failed: %s\n", db_strerror(r));
		goto error_2;
	}
	r = db_create(&db, env, 0);
	if (r != 0) {
		fprintf(stderr, "db_create failed: %s\n", db_strerror(r));
		goto error_2;
	}

	/* Disable compression */
	r = db->set_compression_method(db, TOKU_NO_COMPRESSION);
	if (r != 0) {
		fprintf(stderr, "db->set_compression_method failed: %s\n",
			db_strerror(r));
		goto error_3;
	}

	int open_flags = DB_CREATE;
	r = db->open(db, NULL, "data.bdb", NULL, DB_BTREE, open_flags, 0644);
	if (r != 0) {
		fprintf(stderr, "db->open failed: %s\n", db_strerror(r));
		goto error_3;
	}

	TOKU_COMPRESSION_METHOD compression;
	r = db->get_compression_method(db, &compression);
	if (r != 0) {
		fprintf(stderr, "db->set_compression_method failed: %s\n",
			db_strerror(r));
		goto error_3;
	}
	fprintf(stderr, "TokuKV compression method: %d\n", compression);

	tokukv->env = env;
	tokukv->db = db;
	return (struct nb_db *) tokukv;

error_3:
	db->close(db, 0);
error_2:
	env->close(env, 0);
error_1:
	return NULL;
}

static void
db_tokukv_dtor(struct nb_db *db)
{
	struct nb_db_tokukv *tokukv = (struct nb_db_tokukv *) db;

	tokukv->db->close(tokukv->db, 0);
	tokukv->env->close(tokukv->env, 0);
	tokukv->env = NULL;
	tokukv->db = NULL;
	free(tokukv);
}

static int
db_tokukv_replace(struct nb_db *db, const struct nb_slice *key,
		     const struct nb_slice *val)
{
	struct nb_db_tokukv *tokukv = (struct nb_db_tokukv *) db;

	DBT dbkey, dbval;
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbval, 0, sizeof(dbval));

	dbkey.data = (void *) key->data;
	dbkey.size = key->size;
	dbval.data = (void *) val->data;
	dbval.size = val->size;

	int put_flags = 0; // DB_OVERWRITE_DUP;
	int r = tokukv->db->put(tokukv->db, NULL, &dbkey, &dbval,
				    put_flags);
	if (r == DB_NOTFOUND) {
		fprintf(stderr, "db->get() failed: %d\n", r);
		return -1;
	}

	return 0;
}

static int
db_tokukv_delete(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_tokukv *tokukv = (struct nb_db_tokukv *) db;

	DBT dbkey;
	memset(&dbkey, 0, sizeof(dbkey));

	dbkey.data = (void *) key->data;
	dbkey.size = key->size;

	int r = tokukv->db->del(tokukv->db, NULL, &dbkey, 0);
	if (r == DB_NOTFOUND) {
		fprintf(stderr, "db->del() failed: %d\n", r);
		return -1;
	}

	return 0;
}

static int
db_tokukv_select(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_tokukv *tokukv = (struct nb_db_tokukv *) db;

	DBT dbkey, dbval;
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbval, 0, sizeof(dbval));

	dbkey.data = (void *) key->data;
	dbkey.size = key->size;

	int r = tokukv->db->get(tokukv->db, NULL, &dbkey, &dbval, 0);
	if (r == DB_NOTFOUND) {
		fprintf(stderr, "db->get() failed: %d\n", r);
		return -1;
	}

	return 0;
}

struct nb_db_if nb_db_tokukv =
{
	.name       = "tokukv",
	.ctor       = db_tokukv_ctor,
	.dtor       = db_tokukv_dtor,
	.replace_cb = db_tokukv_replace,
	.delete_cb  = db_tokukv_delete,
	.select_cb  = db_tokukv_select,
};
