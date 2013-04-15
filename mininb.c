#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "dbif.h"
#include "options.h"
#include "random.h"
#include "time.h"
#include "histogram.h"

static int
bench(struct nb_options *opts)
{
	int rc = 0;

	rc++;
	struct nb_db_if *db_if = nb_db_match(opts->driver);
	if (db_if == NULL) {
		fprintf(stderr, "Driver '%s'' is not found!", opts->driver);
		goto error_1;
	}

	rc++;
	struct nb_db *db = db_if->ctor(opts);
	if (db == NULL) {
		fprintf(stderr, "driver::new failed\n");
		goto error_2;
	}

	char *key = malloc(opts->key_size);
	if (key == NULL) {
		fprintf(stderr, "key malloc failed\n");
		goto error_3;
	}

	struct random random;
	if (random_create(&random, opts->keys_filename) != 0) {
		fprintf(stderr, "random_create failed\n");
		fprintf(stderr, "Please generate random file using dd"
			"dd if=/dev/urandom of=keys.bin bs=1M count=100\n");
		goto error_4;
	}

	struct nb_histogram *hist = nb_histogram_new(6);
	if (hist == NULL) {
		fprintf(stderr, "nb_histogram_new() failed\n");
		goto error_5;
	}

	size_t prev_count = 0;
	size_t total_count = 0;

	fprintf(stderr, "Benchmarking...");
	for (size_t kk = 0; kk < opts->bench_count; kk++) {
		struct nb_slice skey;
		struct nb_slice sval;
#if STR_KEYS
		char keybuf[1024];
		sprintf(keybuf, "%u", *(uint32_t *) key);

		skey.size = strlen(keybuf);
		skey.data = keybuf;
		sval.size = strlen(keybuf);
		sval.data = keybuf;

#else
		if (random_next(&random, key, opts->key_size) != 0) {
			fprintf(stderr, "random_next failed\n");
			return 1;
		}

		skey.size = opts->key_size;
		skey.data = key;
		sval.size = opts->value_size;
		sval.data = key;
#endif
		double t0 = nb_clock();
		switch (opts->type) {
		case NB_BENCH_GET:
			if (db_if->select_cb(db, &skey) != 0) {
				fprintf(stdout, "key: %.*s\n",
					(int) skey.size, (char *) skey.data);
				fprintf(stderr, "Select failed :(\n");
				return 1;
			}
			break;
		case NB_BENCH_PUT:
			if (db_if->replace_cb(db, &skey, &sval) != 0) {
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

	db_if->dtor(db);

	nb_histogram_delete(hist);
	random_destroy(&random);

	return 0;

error_5:
	random_destroy(&random);
error_4:
	free(key);

error_3:
	db_if->dtor(db);
error_2:

error_1:
	return rc;
}

int
main(int argc, char *argv[])
{
	struct nb_options opts = {
		.driver = "cascadedb",
		.root  = "./nb",
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
		int r = random_shuffle(opts.keys_filename,
				       opts.key_size, opts.bench_count);
		if (r != 0) {
			return -1;
		}
		fprintf(stderr, "\r" "Shuffling file..." " " "ok" "\n");

		return 0;
	}
}
