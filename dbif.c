
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

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dbif.h"
#if defined(HAVE_LEVELDB)
#include "db_leveldb.h"
#endif
#if defined(HAVE_NESSDB_V1) || defined(HAVE_NESSDB_V2)
#include "db_nessdb.h"
#endif
#if defined(HAVE_CASCADEDB)
#include "db_cascadedb.h"
#endif
#if defined(HAVE_BERKELEYDB)
#include "db_berkeleydb.h"
#endif
#if defined(HAVE_KYOTOCABINET)
#include "db_kyotocabinet.h"
#endif

struct nb_db_if *nb_dbs[] =
{
#if defined(HAVE_LEVELDB)
	&nb_db_leveldb,
#endif
#if defined(HAVE_NESSDB_V1) || defined(HAVE_NESSDB_V2)
	&nb_db_nessdb,
#endif
#if defined(HAVE_CASCADEDB)
	&nb_db_cascadedb,
#endif
#if defined(HAVE_BERKELEYDB)
	&nb_db_berkeleydb,
#endif
#if defined(HAVE_KYOTOCABINET)
	&nb_db_kyotocabinet,
#endif
	NULL,
};

struct nb_db_if *nb_db_match(const char *name)
{
	int i = 0;
	while (nb_dbs[i]) {
		if (strcmp(nb_dbs[i]->name, name) == 0)
			return nb_dbs[i];
		i++;
	}
	return NULL;
}
