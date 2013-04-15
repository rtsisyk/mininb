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

#include "db_berkeleydb.h"

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

typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
#include <db4.8/db.h>

#include "config.h"
#include "db.h"

struct nb_db_berkeleydb {
	DB_ENV *env;
	DB *db;
};

static struct nb_db *
db_berkeleydb_ctor(const struct nb_options *opts)
{
	struct nb_db_berkeleydb *berkeleydb = malloc(sizeof(*berkeleydb));
	if (berkeleydb == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*berkeleydb));
		goto error_1;
	}

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 1, "%s/%s", opts->root, "berkeleydb");
	path[FILENAME_MAX - 1] = 0;

	fprintf(stderr, "BerkeleyDB new: path=%s\n", path);

	int r;
	r = mkdir(path, 0777);
	if (r != 0 && errno != EEXIST) {
		fprintf(stderr, "mkdir: %d\n", r);
		goto error_1;
	}

	DB_ENV *env;
	DB *db;

	int env_flags = DB_REGION_INIT;
	r = db_env_create(&env, 0);
	if (r != 0) {
		fprintf(stderr, "db_env_create: %d\n", r);
		goto error_1;
	}
	r = env->set_cachesize(env, 0, 0, 1);
	if (r != 0) {
		fprintf(stderr, "set_cachesize: %d\n", r);
		goto error_2;
	}
	env_flags |= DB_TXN_WRITE_NOSYNC;
	r = env->set_flags(env, env_flags, 1);
	if (r != 0) {
		fprintf(stderr, "set_flags: %d\n", r);
		goto error_2;
	}
	int log_flags = DB_LOG_AUTO_REMOVE;
	r = env->log_set_config(env, log_flags, 1);

	int env_open_flags = DB_CREATE|DB_INIT_MPOOL;
	r = env->open(env, path, env_open_flags, 0666);
	if (r != 0) {
		fprintf(stderr, "env->open: %d\n", r);
		goto error_2;
	}
	r = db_create(&db, env, 0);
	if (r != 0) {
		fprintf(stderr, "db_create: %d\n", r);
		goto error_2;
	}
	int open_flags = DB_CREATE;
	r = db->open(db, NULL, "data.bdb", NULL, DB_BTREE, open_flags, 0664);
	if (r != 0) {
		fprintf(stderr, "db->open: %d\n", r);
		goto error_3;
	}

	berkeleydb->env = env;
	berkeleydb->db = db;
	return (struct nb_db *) berkeleydb;

error_3:
	db->close(db, 0);
error_2:
	env->close(env, 0);
error_1:
	return NULL;
}

static void
db_berkeleydb_dtor(struct nb_db *db)
{
	struct nb_db_berkeleydb *berkeleydb = (struct nb_db_berkeleydb *) db;

	berkeleydb->db->close(berkeleydb->db, 0);
	berkeleydb->env->close(berkeleydb->env, 0);
	berkeleydb->env = NULL;
	berkeleydb->db = NULL;
	free(berkeleydb);
}

static int
db_berkeleydb_replace(struct nb_db *db, const struct nb_slice *key,
		     const struct nb_slice *val)
{
	struct nb_db_berkeleydb *berkeleydb = (struct nb_db_berkeleydb *) db;

	DBT dbkey, dbval;
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbval, 0, sizeof(dbval));

	dbkey.data = (void *) key->data;
	dbkey.size = key->size;
	dbval.data = (void *) val->data;
	dbval.size = val->size;

	int put_flags = DB_OVERWRITE_DUP;
	int r = berkeleydb->db->put(berkeleydb->db, NULL, &dbkey, &dbval,
				    put_flags);
	if (r == DB_NOTFOUND) {
		fprintf(stderr, "db->get() failed: %d\n", r);
		return -1;
	}

	return 0;
}

static int
db_berkeleydb_delete(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_berkeleydb *berkeleydb = (struct nb_db_berkeleydb *) db;

	DBT dbkey;
	memset(&dbkey, 0, sizeof(dbkey));

	dbkey.data = (void *) key->data;
	dbkey.size = key->size;

	int r = berkeleydb->db->del(berkeleydb->db, NULL, &dbkey, 0);
	if (r == DB_NOTFOUND) {
		fprintf(stderr, "db->del() failed: %d\n", r);
		return -1;
	}

	return 0;
}

static int
db_berkeleydb_select(struct nb_db *db, const struct nb_slice *key)
{
	struct nb_db_berkeleydb *berkeleydb = (struct nb_db_berkeleydb *) db;

	DBT dbkey, dbval;
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbval, 0, sizeof(dbval));

	dbkey.data = (void *) key->data;
	dbkey.size = key->size;

	int r = berkeleydb->db->get(berkeleydb->db, NULL, &dbkey, &dbval, 0);
	if (r == DB_NOTFOUND) {
		fprintf(stderr, "db->get() failed: %d\n", r);
		return -1;
	}

	return 0;
}

struct nb_db_if nb_db_berkeleydb =
{
	.name       = "berkeleydb",
	.ctor       = db_berkeleydb_ctor,
	.dtor       = db_berkeleydb_dtor,
	.replace_cb = db_berkeleydb_replace,
	.delete_cb  = db_berkeleydb_delete,
	.select_cb  = db_berkeleydb_select,
};
