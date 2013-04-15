#include "time.h"

#include <time.h>

double
nb_clock(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return t.tv_sec + 1e-9 * t.tv_nsec;
}

double
nb_now(void)
{
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return t.tv_sec + 1e-9 * t.tv_nsec;
}
