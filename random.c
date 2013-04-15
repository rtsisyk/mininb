#include "random.h"

#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int
random_open_file(struct random *rnd, const char *filename, bool rw)
{
	int rc = 0;
	int r;

	rc--;
	int flags = rw ? O_RDWR : O_RDONLY;
	int fd = open(filename, flags);
	if (fd == -1) {
		perror("open");
		goto error_1;
	}

	struct stat file_stat;
	r = fstat(fd, &file_stat);
	if (r != 0) {
		perror("fstat");
		goto error_2;
	}

	rc--;
	int flags2 = rw ? (PROT_READ|PROT_WRITE) : PROT_READ;
	void *map = mmap(NULL, file_stat.st_size, flags2,
			 MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		perror("mmap failed");
		goto error_2;
	}

	rnd->fd = fd;
	rnd->map = map;
	rnd->cur = 0;
	rnd->end = file_stat.st_size;

	return 0;

error_2:
	close(fd);
error_1:
	return rc;
}

static int
random_close_file(struct random *random)
{
	int rc = 0;
	int r;

	rc--;
	r = munmap(random->map, random->end);
	if (r != 0) {
		perror("munmap");
		goto error_2;
	}

	rc--;
	r = close(random->fd);
	if (r != 0) {
		perror("close");
		goto error_1;
	}

	return 0;

error_2:
error_1:
	return rc;
}

int
random_create(struct random *random, const char *filename)
{
	int rc = 0;
	memset(random, 0, sizeof(*random));

	rc--;
	int r = random_open_file(random, filename, false);
	if (r != 0)
		goto error_1;

	rc--;
	r = posix_madvise(random->map, random->end, POSIX_MADV_SEQUENTIAL);
	if (r != 0) {
		perror("madvise");
		goto error_2;
	}

	return 0;

error_2:
	random_close_file(random);
error_1:
	return rc;
}

void
random_destroy(struct random *random)
{
	random_close_file(random);
}

int
random_next(struct random *random, char *key, size_t key_size)
{
	if (random->cur + key_size >= random->end)
		return 1;

	memcpy(key, (char *) random->map + random->cur, key_size);
	random->cur += key_size;

	return 0;
}

int
random_shuffle(const char *filename, size_t bs, size_t count)
{
	struct random random;
	memset(&random, 0, sizeof(random));

	int rc = 0;
	int r;

	rc--;
	r = random_open_file(&random, filename, true);
	if (r != 0) {
		goto error_1;
	}

	size_t n = random.end / bs;

	if (n == 0) {
		goto skip;
	}

	if (n > count) {
		n = count;
	}

	rc--;
	char *buf = malloc(bs);
	if (buf == NULL) {
		perror("malloc failed");
		goto error_2;
	}

	for (size_t i = 0; i < n - 1; i++)
	{
		  size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
		  char *a = (char *) random.map + bs * i;
		  char *b = (char *) random.map + bs * j;

		  memcpy(buf, a, bs);
		  memcpy(a, b, bs);
		  memcpy(b, buf, bs);
	}

	free(buf);
	rc--;
	r = msync(random.map, random.end, MS_SYNC);
	if (r != 0) {
		perror("msync");
		goto error_2;
	}

skip:
	r = random_close_file(&random);
	if (r != 0) {
		goto error_1;
	}

	return 0;

error_2:
	random_close_file(&random);
error_1:
	return rc;
}
