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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "nb_plugin.h"
#include "nb_opts.h"
#include "nb_random.h"
#include "nb_time.h"
#include "nb_histogram.h"

static int
bench(struct nb_opts *opts)
{
	int rc = 0;

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

	char *keybuf = malloc(opts->key_size);
	if (keybuf == NULL) {
		fprintf(stderr, "key malloc failed\n");
		goto error_3;
	}

	char *valbuf = malloc(opts->value_size);
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
	for (size_t kk = 0; kk < opts->bench_count; kk++) {
		const void *key;
		size_t key_len;
		const void *val;
		size_t val_len;

		if (nb_random_next(&random, keybuf, opts->key_size) != 0) {
			fprintf(stderr, "random_next failed\n");
			return 1;
		}

		key = keybuf;
		key_len = opts->key_size;
		val = valbuf;
		val_len = opts->value_size;

		double t0 = nb_clock();
		switch (opts->type) {
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
		}

		double t1 = nb_clock();
		double td = t1 - t0;
		nb_histogram_add(hist, td);

		prev_count++;
		total_count++;

		if (prev_count < opts->report_request_interval)
			continue;

		fprintf(stderr, "\r%zu ops done...", total_count);
		prev_count = 0;
	}

	fprintf(stderr, "\r%zu ops done...\n", opts->bench_count);

	fprintf(stdout, "Histogram:\n");
	nb_histogram_dump(hist, stdout);

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

int
main(int argc, char *argv[])
{
	struct nb_opts opts = {
		.root = "./nb",
		.driver = "leveldb",
		.key_size = 16,
		.value_size = 100,
		.report_request_interval = 100000,
		.bench_count = 1000000,
		.type = NB_BENCH_PUT,
		.keys_filename = "keys.bin"
	};

	if (argc >= 2) {
		opts.driver = argv[1];
	}

	if (argc >= 3) {
		opts.bench_count = atol(argv[2]);
	}

	int action = 0;
	if (argc >= 4) {
		action = atoi(argv[3]);
	}

	fprintf(stderr, "Driver: %s\n", opts.driver);
	fprintf(stderr, "Count : %zu\n", opts.bench_count);

	char path[FILENAME_MAX];
	snprintf(path, FILENAME_MAX - 1, "%s/%s", opts.root, opts.driver);
	path[FILENAME_MAX - 1] = 0;
	opts.db_opts.path = path;

	if (action == 0) {
		opts.type = NB_BENCH_PUT;
		fprintf(stderr, "PUT\n");
		int r = bench(&opts);
		if (r != 0) {
			return -1;
		}

		return 0;
	} else if (action == 1){
		opts.type = NB_BENCH_GET;
		fprintf(stderr, "GET\n");
		int r = bench(&opts);
		if (r != 0) {
			return -1;
		}

		return 0;
	} else if (action == 2) {
		fprintf(stderr, "Shuffling file...");
		int r = nb_random_shuffle(opts.keys_filename,
				       opts.key_size, opts.bench_count);
		if (r != 0) {
			return -1;
		}
		fprintf(stderr, "\r" "Shuffling file..." " " "ok" "\n");

		return 0;
	}
}
