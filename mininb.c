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
#include <getopt.h>

#include "nb_random.h"
#include "nb_engine.h"

static int
action_get(struct nb_opts *opts)
{
	return nb_engine_run(opts, NB_BENCH_GET);
}

static int
action_put(struct nb_opts *opts)
{
	return nb_engine_run(opts, NB_BENCH_PUT);
}

static int
action_shuffle(struct nb_opts *opts)
{
	fprintf(stderr, "Shuffling file...");
	int r = nb_random_shuffle(opts->keys_filename,
				  opts->key_len, opts->count);
	if (r != 0) {
		return -1;
	}
	fprintf(stderr, "\r" "Shuffling file..." " " "ok" "\n");

	return 0;
}

static struct action {
	int (*action)(struct nb_opts *opts);
	const char *name;
	const char *descr;
} ACTIONS[] = {
	{ action_get,     "get",      "GET benchmark"},
	{ action_put,     "put",      "PUT benchmark"},
	{ action_shuffle, "shuffle",  "Shuffle keys file"},
	{ NULL,           NULL,       NULL }
};

struct nb_opts opts = {
	.path   = "./nb",
	.driver = "leveldb",
	.keys_filename = "keys.bin",
	.key_len = 16,
	.val_len = 100,
	.report_interval = 10000,
	.count = 100000,
};

void
usage(void) {
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t--action=");
	for (int i = 0; ACTIONS[i].action != NULL; i++) {
		if (i != 0) {
			fprintf(stderr, "|%s", ACTIONS[i].name);
		} else {
			fprintf(stderr, "%s", ACTIONS[i].name);
		}
	}
	fprintf(stderr, " - an action to execute\n");

	for (int i = 0; ACTIONS[i].action != NULL; i++) {
		fprintf(stderr, "\t         %s - %s\n",
			ACTIONS[i].name, ACTIONS[i].descr);
	}
	fprintf(stderr, "\t--path='%s' - a path where database "
		"files will be stored\n", opts.path);
	fprintf(stderr, "\t--driver='%s' - database driver name\n",
		opts.driver);
	fprintf(stderr, "\t--klen=%zu - key length in bytes\n",
		opts.key_len);
	fprintf(stderr, "\t--vlen=%zu - value length in bytes\n",
		opts.val_len);
	fprintf(stderr, "\t--keys=%s - path to a binary file with keys\n",
		opts.keys_filename);
	fprintf(stderr, "\t--report-interval=%zu - report interval (records)\n",
		opts.report_interval);
	fprintf(stderr, "\t--count=%zu - number of records\n",
		opts.count);

	fprintf(stderr, "\n\n");
	fprintf(stderr, "Example:\n");
	fprintf (stderr, "# Benchmark PUT operation\n");
	fprintf(stderr, "./mininb --count=1000000 --action=put\n");
	fprintf (stderr, "# Shuffle keys\n");
	fprintf(stderr, "./mininb --count=1000000 --action=shuffle\n");
	fprintf (stderr, "# Benchmark GET operation\n");
	fprintf(stderr, "./mininb --count=1000000 --action=get\n");
}

int
main(int argc, char *argv[])
{
	static struct option options[] = {
		{"action",              required_argument, NULL, 'a'},
		{"path",                required_argument, NULL, 'p'},
		{"driver",              required_argument, NULL, 'd'},
		{"klen",                required_argument, NULL, 'k'},
		{"vlen",                required_argument, NULL, 'v'},
		{"keys",                required_argument, NULL, 'i'},
		{"report-interval",     required_argument, NULL, 'r'},
		{"count",               required_argument, NULL, 'c'},
		{0,                     0,                 0,     0 }
	};

	struct action *action = &ACTIONS[0];

	while (1) {
		int option_index = 0;

		int c = getopt_long(argc, argv, "a:p:d:k:v:i:r:c:",
				    options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'a':
			action = NULL;
			for (int i = 0; ACTIONS[i].action != NULL; i++) {
				if (strcmp(ACTIONS[i].name, optarg) == 0) {
					action = &ACTIONS[i];
				}
			}
			break;
		case 'p':
			opts.path = optarg;
			break;
		case 'd':
			opts.driver = optarg;
			break;
		case 'k':
			opts.key_len = atol(optarg);
			break;
		case 'v':
			opts.val_len = atol(optarg);
			break;
		case 'i':
			opts.keys_filename = optarg;
			break;
		case 'r':
			opts.report_interval = atol(optarg);
			break;
		case 'c':
			opts.count = atol(optarg);
			break;
		default:
			fprintf(stderr, "Invalid option: %x\n", c);
			usage();
			return -1;
		}
	}

	if (action == NULL) {
		fprintf(stderr, "Invalid action!\n\n");
		usage();
		return -1;
	}

	fprintf(stderr, "Mini NoSQL Benchmark\n");
	fprintf(stderr, "====================\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Path: %s\n",   opts.path);
	fprintf(stderr, "Keys File: %s\n", opts.keys_filename);

	fprintf(stderr, "Action: %s\n", action->name);
	fprintf(stderr, "Driver: %s\n", opts.driver);
	fprintf(stderr, "Key Len: %zu\n", opts.key_len);
	fprintf(stderr, "Val Len: %zu\n", opts.val_len);
	fprintf(stderr, "Count: %zu\n", opts.count);

	return action->action(&opts);
}
