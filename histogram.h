#ifndef NB_HISTOGRAM_H_INCLUDED
#define NB_HISTOGRAM_H_INCLUDED

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

struct nb_histogram;

struct nb_histogram *
nb_histogram_new(int power);

void
nb_histogram_delete(struct nb_histogram *hist);

void
nb_histogram_add(struct nb_histogram *hist, double val);

void
nb_histogram_clear(struct nb_histogram *hist);

void
nb_histogram_dump(const struct nb_histogram *hist, FILE *file);

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* NB_HISTOGRAM_H_INCLUDED */
