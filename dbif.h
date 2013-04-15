#ifndef NB_DB_H_INCLUDED
#define NB_DB_H_INCLUDED

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
#include <stdbool.h>

#include "options.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

struct nb_db;

struct nb_slice {
	const void *data;
	size_t size;
};

typedef int (*nb_db_cb1_t)(struct nb_db *db, const struct nb_slice *key);

typedef int (*nb_db_cb2_t)(struct nb_db *db, const struct nb_slice *key,
			   const struct nb_slice *val);

struct nb_db_if {
	const char *name;
	struct nb_db *(*ctor)(const struct nb_options *opts);
	void (*dtor)(struct nb_db *);
	nb_db_cb2_t replace_cb;
	nb_db_cb1_t delete_cb;
	nb_db_cb1_t select_cb;
};

struct nb_db {
	struct nb_db_if *dif;
};

extern struct nb_db_if *nb_dbs[];

struct nb_db_if *nb_db_match(const char *name);

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* NB_DB_H_INCLUDED */
