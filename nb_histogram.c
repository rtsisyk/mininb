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

#include "nb_histogram.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

/* Numbers were derived from leveldb benchmark to be compatible with it */
static const double NB_HISTOGRAM_BUCKETS[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 25, 30, 35, 40, 45,
	50, 60, 70, 80, 90, 100, 120, 140, 160, 180, 200, 250, 300, 350, 400, 450,
	500, 600, 700, 800, 900, 1000, 1200, 1400, 1600, 1800, 2000, 2500, 3000,
	3500, 4000, 4500, 5000, 6000, 7000, 8000, 9000, 10000, 12000, 14000,
	16000, 18000, 20000, 25000, 30000, 35000, 40000, 45000, 50000, 60000,
	70000, 80000, 90000, 100000, 120000, 140000, 160000, 180000, 200000,
	250000, 300000, 350000, 400000, 450000, 500000, 600000, 700000, 800000,
	900000, 1000000, 1200000, 1400000, 1600000, 1800000, 2000000, 2500000,
	3000000, 3500000, 4000000, 4500000, 5000000, 6000000, 7000000, 8000000,
	9000000, 10000000, 12000000, 14000000, 16000000, 18000000, 20000000,
	25000000, 30000000, 35000000, 40000000, 45000000, 50000000, 60000000,
	70000000, 80000000, 90000000, 100000000, 120000000, 140000000, 160000000,
	180000000, 200000000, 250000000, 300000000, 350000000, 400000000,
	450000000, 500000000, 600000000, 700000000, 800000000, 900000000,
	1000000000, 1200000000, 1400000000, 1600000000, 1800000000, 2000000000,
	2500000000.0, 3000000000.0, 3500000000.0, 4000000000.0, 4500000000.0,
	5000000000.0, 6000000000.0, 7000000000.0, 8000000000.0, 9000000000.0,
	1e200, INFINITY,
};

enum {
	NB_HISTOGRAM_BUCKETS_COUNT =
	sizeof(NB_HISTOGRAM_BUCKETS) / sizeof(*NB_HISTOGRAM_BUCKETS)
};

struct nb_histogram {
	double min;
	double max;
	double sum;
	double sumsq;
	size_t size;
	size_t buckets[NB_HISTOGRAM_BUCKETS_COUNT];
	int power;
};

struct nb_histogram *
nb_histogram_new(int power)
{
	struct nb_histogram *hist = malloc(sizeof(*hist));
	if (hist == NULL) {
		fprintf(stderr, "malloc(%zu) failed", sizeof(*hist));
		goto error;
	}

	hist->power = power;
	nb_histogram_clear(hist);
	return hist;
error:
	return NULL;
}

void
nb_histogram_delete(struct nb_histogram *hist)
{
	free(hist);
}

void
nb_histogram_add(struct nb_histogram *hist, double val)
{
	val *= pow(10, hist->power);

	size_t begin = 0;
	size_t end = NB_HISTOGRAM_BUCKETS_COUNT-1;
	size_t mid;

	while(1) {
		mid = begin / 2 + end / 2;
		if (mid == begin) {
			for (mid = end; mid > begin; mid--) {
				if (NB_HISTOGRAM_BUCKETS[mid-1] < val) {
					break;
				}
			}
			break;
		}

		if (NB_HISTOGRAM_BUCKETS[mid-1] < val) {
			begin = mid;
		} else {
			end = mid;
		}
	};

	if (hist->min > val) {
		hist->min = val;
	}
	if (hist->max < val) {
		hist->max = val;
	}

	hist->sum += val;
	hist->sumsq += val * val;

	hist->buckets[mid]++;
	hist->size++;
}

void
nb_histogram_clear(struct nb_histogram *hist)
{
	int power = hist->power;
	memset(hist, 0, sizeof(*hist));
	hist->power = power;
	hist->min = INFINITY;
}

static double
nb_histogram_percentile(const struct nb_histogram *hist, double p)
{
	size_t threshold = (size_t) hist->size * p;
	size_t count = 0;

	for (size_t b = 0; b < NB_HISTOGRAM_BUCKETS_COUNT; b++) {
		count += hist->buckets[b];
		if (count >= threshold) {
			size_t left = count - hist->buckets[b];
			size_t right = count;
			double left_val = (b>0) ? NB_HISTOGRAM_BUCKETS[b-1] : 0;
			double right_val = NB_HISTOGRAM_BUCKETS[b];
			double scale = (double) (threshold - left) /
				       (right - left);

			double r = left_val + (right_val - left_val) * scale;
			if (r < hist->min) {
				return hist->min;
			} else if (r > hist->max) {
				return hist->max;
			} else {
				return r;
			}
		}
	}

	return hist->max;
}

void
nb_histogram_dump(const struct nb_histogram *hist, FILE *file,
		  double *percentiles, size_t percentiles_size)
{
	assert (hist->size > 0);
	fprintf(file, "[%7s, %7s)\t%11s\t%7s\n", "t min", "t max",
		"ops count", "%");
	fprintf(file, "--------------------------------------------------\n");
	for (size_t i = 0; i < NB_HISTOGRAM_BUCKETS_COUNT; i++) {
		if (hist->buckets[i] == 0.0)
			continue;
		double percents = (double) hist->buckets[i] / hist->size;
		fprintf(file, "[%7.0lf, %7.0lf)\t%11zu\t%7.2lf ",
			(i > 0) ? NB_HISTOGRAM_BUCKETS[i-1] : 0.0,
			NB_HISTOGRAM_BUCKETS[i],
			hist->buckets[i],
			percents * 1e2);

		unsigned marks = (unsigned) percents * 1e2 / 5.0;
		for (unsigned m = 0; m < marks; m++) {
			fputc('#', file);
		}

		fputc('\n', file);
	}

	double avg_latency = hist->sum / hist->size;

	fprintf(file, "--------------------------------------------------\n");
	fprintf(file, "Total:%5s%7.0lf\t%11zu\t   100%%\n", "",
		hist->sum, hist->size);
	fprintf(file, "Min latency       : %7.6lf * 1e%d sec/op\n",
		hist->min, -hist->power);
	fprintf(file, "Avg latency       : %7.6lf * 1e%d sec/op\n",
		avg_latency, -hist->power);
	fprintf(file, "Max latency       : %7.6lf * 1e%d sec/op\n",
		hist->max, -hist->power);

	for (size_t i = 0; i < percentiles_size; i++) {
		double p = percentiles[i];
		fprintf(file, "%-2.4lf%%  latency  : %7.6lf * 1e%d sec/op\n",
			p*1e2, nb_histogram_percentile(hist, p), -hist->power);
	}
	fprintf(file, "Avg throughput    : %7.0lf ops/sec\n",
		(double) hist->size / (hist->sum * pow(10, -hist->power)));
}

