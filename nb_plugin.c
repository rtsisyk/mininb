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

#include "nb_plugin.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h> /* PATH_MAX */
#include <dlfcn.h>

typedef const struct nb_db_if *
(*nb_db_loader_t)(void);

struct nb_plugin *
nb_plugin_load(const char *name)
{
	struct nb_plugin *plugin = malloc(sizeof(*plugin));
	if (plugin == NULL) {
		fprintf(stderr, "malloc(%zu) failed\n", sizeof(*plugin));
		goto error_1;
	}

	char path[PATH_MAX];
	snprintf(path, PATH_MAX - 1, "plugins/%s/libnb_db_%s.so", name, name);
	path[PATH_MAX - 1] = 0;
	void *handle = dlopen(path, RTLD_NOW);
	if (handle == NULL) {
		fprintf(stderr, "Can not load '%s' file: %s\n",
			path, dlerror());
		goto error_2;
	}

	char symbol[PATH_MAX];
	snprintf(symbol, PATH_MAX - 1, "nb_db_%s_plugin", name);
	symbol[PATH_MAX - 1] = 0;

	/* Clear any existing error */
	dlerror();

	nb_db_loader_t loader = (nb_db_loader_t) dlsym(handle, symbol);
	if (loader == NULL) {
		fprintf(stderr, "Invalid plugin '%s': %s\n",
			name, dlerror());
		goto error_3;
	}

	plugin->pif = loader();
	if (strcmp(plugin->pif->name, name) != 0) {
		fprintf(stderr, "Invalid plugin '%s'\n", name);
		goto error_3;
	}
	plugin->handle = handle;

	return plugin;

error_3:
	dlclose(handle);
error_2:
	free(plugin);
error_1:
	return NULL;
}

void
nb_plugin_unload(struct nb_plugin *plugin)
{
	dlclose(plugin->handle);
	free(plugin);
}
