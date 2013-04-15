#ifndef NB_RANDOM_H_INCLUDED
#define NB_RANDOM_H_INCLUDED

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

struct random {
	int fd;
	void *map;
	size_t cur;
	size_t end;
};

int
random_create(struct random *random, const char *filename);

void
random_destroy(struct random *random);

int
random_next(struct random *random, char *key, size_t key_size);

int
random_shuffle(const char *filename, size_t bs, size_t count);

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif /* NB_RANDOM_H_INCLUDED */
