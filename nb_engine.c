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

#include "nb_engine.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "nb_plugin.h"
#include "nb_histogram.h"
#include "nb_random.h"
#include "nb_time.h"

int
nb_engine_run(struct nb_opts *opts, enum nb_bench_type bench_type)
{
	int rc = 0;

	char path[PATH_MAX];
	snprintf(path, PATH_MAX - 1, "%s/%s", opts->path, opts->driver);
	path[PATH_MAX - 1] = 0;
	opts->db_opts.path = path;

	rc++;
	struct nb_plugin *plugin = nb_plugin_load(opts->driver);
	if (plugin == NULL) {
		fprintf(stderr, "Driver '%s' is not found!\n", opts->driver);
		goto error_1;
	}

	rc++;
	struct nb_db *db = plugin->pif->open(&opts->db_opts);
	if (db == NULL) {
		fprintf(stderr, "driver::new failed\n");
		goto error_2;
	}

	char *keybuf = malloc(opts->key_len);
	if (keybuf == NULL) {
		fprintf(stderr, "key malloc failed\n");
		goto error_3;
	}

	char *valbuf = malloc(opts->val_len);
	if (valbuf == NULL) {
		fprintf(stderr, "val malloc failed\n");
		goto error_4;
	}

	struct nb_random random;
	if (nb_random_create(&random, opts->keys_filename) != 0) {
		fprintf(stderr, "random_create failed\n");
		fprintf(stderr, "Please generate random file using dd:\n"
			"dd if=/dev/urandom of=keys.bin bs=1M count=100\n");
		goto error_5;
	}

	struct nb_histogram *hist = nb_histogram_new(6);
	if (hist == NULL) {
		fprintf(stderr, "nb_histogram_new() failed\n");
		goto error_6;
	}

	size_t prev_count = 0;
	size_t total_count = 0;

	fprintf(stderr, "Benchmarking...");
	for (size_t kk = 0; kk < opts->count; kk++) {
		const void *key;
		size_t key_len;
		const void *val;
		size_t val_len;

		if (nb_random_next(&random, keybuf, opts->key_len) != 0) {
			fprintf(stderr, "random_next failed\n");
			return 1;
		}

		key = keybuf;
		key_len = opts->key_len;
		val = valbuf;
		val_len = opts->val_len;

		double t0 = nb_clock();
		switch (bench_type) {
		case NB_BENCH_GET:
			if (plugin->pif->select(db, key, key_len,
						NULL, NULL) != 0) {
				fprintf(stdout, "key: %.*s\n",
					(int) key_len, (char *) key);
				fprintf(stderr, "Select failed :(\n");
				return 1;
			}
			break;
		case NB_BENCH_PUT:
			if (plugin->pif->replace(db, key, key_len,
						 val, val_len) != 0) {
				fprintf(stderr, "Replace failed :(\n");
				goto error_5;
			}
			break;
		default:
			assert(0);
		}

		double t1 = nb_clock();
		double td = t1 - t0;
		nb_histogram_add(hist, td);

		prev_count++;
		total_count++;

		if (prev_count < opts->report_interval)
			continue;

		fprintf(stderr, "\r%zu ops done...", total_count);
		prev_count = 0;
	}

	fprintf(stderr, "\r%zu ops done...\n", opts->count);

	double percentiles[] = { 0.05, 0.50, 0.95, 0.96, 0.97, 0.98, 0.99,
				 0.995, 0.999, 0.9995, 0.9999 };
	size_t percentiles_size = sizeof(percentiles) / sizeof(percentiles[0]);

	fprintf(stdout, "Histogram:\n");
	nb_histogram_dump(hist, stdout, percentiles, percentiles_size);

	plugin->pif->close(db);

	nb_histogram_delete(hist);
	nb_random_destroy(&random);

	free(valbuf);
	free(keybuf);

	nb_plugin_unload(plugin);

	return 0;

error_6:
	nb_random_destroy(&random);
error_5:
	free(valbuf);
error_4:
	free(keybuf);

error_3:
	plugin->pif->close(db);
error_2:
	nb_plugin_unload(plugin);
error_1:
	return rc;
}
