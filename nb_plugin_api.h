#ifndef NB_DB_PLUGIN_API_H_INCLUDED
#define NB_DB_PLUGIN_API_H_INCLUDED

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

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define NB_DB_PLUGIN __attribute__((__visibility__("default")))

struct nb_db {
	const struct nb_db_opts *opts;
};

struct nb_db_opts {
	const char *path;
};

typedef struct nb_db *
(*nb_db_open_t)(const struct nb_db_opts *opts);

typedef void
(*nb_db_close_t)(struct nb_db *db);

typedef int
(*nb_db_replace_t)(struct nb_db *db, const void *key, size_t key_len,
		   const void *val, size_t val_len);

typedef int
(*nb_db_remove_t)(struct nb_db *db, const void *key, size_t key_len);

typedef int
(*nb_db_select_t)(struct nb_db *db, const void *key, size_t key_len,
		  void **pval, size_t *pval_len);

typedef void
(*nb_db_valfree_t)(struct nb_db *db, void *val);

struct nb_db_if {
	const char *name;
	nb_db_open_t open;
	nb_db_close_t close;
	nb_db_replace_t replace;
	nb_db_remove_t remove;
	nb_db_select_t select;
	nb_db_valfree_t valfree;
};

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* NB_DB_PLUGIN_API_H_INCLUDED */
