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

#include "nb_random.h"

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
nb_random_open_file(struct nb_random *rnd, const char *filename, bool rw)
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
nb_random_close_file(struct nb_random *random)
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
nb_random_create(struct nb_random *random, const char *filename)
{
	int rc = 0;
	memset(random, 0, sizeof(*random));

	rc--;
	int r = nb_random_open_file(random, filename, false);
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
	nb_random_close_file(random);
error_1:
	return rc;
}

void
nb_random_destroy(struct nb_random *random)
{
	nb_random_close_file(random);
}

int
nb_random_next(struct nb_random *random, char *key, size_t key_size)
{
	if (random->cur + key_size >= random->end)
		return 1;

	memcpy(key, (char *) random->map + random->cur, key_size);
	random->cur += key_size;

	return 0;
}

int
nb_random_shuffle(const char *filename, size_t bs, size_t count)
{
	struct nb_random random;
	memset(&random, 0, sizeof(random));

	int rc = 0;
	int r;

	rc--;
	r = nb_random_open_file(&random, filename, true);
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
	r = nb_random_close_file(&random);
	if (r != 0) {
		goto error_1;
	}

	return 0;

error_2:
	nb_random_close_file(&random);
error_1:
	return rc;
}
