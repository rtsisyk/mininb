#ifndef NB_RANDOM_H_INCLUDED
#define NB_RANDOM_H_INCLUDED

#include <stddef.h>

struct nb_random {
	int fd;
	void *map;
	size_t cur;
	size_t end;
};

int
nb_random_create(struct nb_random *random, const char *filename);

void
nb_random_destroy(struct nb_random *random);

int
nb_random_next(struct nb_random *random, char *key, size_t key_size);

int
nb_random_shuffle(const char *filename, size_t bs, size_t count);

#endif /* NB_RANDOM_H_INCLUDED */
