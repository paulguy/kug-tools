#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "kugfile.h"
#include "kugiter.h"

void usage();

int main(int argc, char **argv) {
	char filename[KUG_BUFFERSIZE];
	char basedir[FILENAME_MAX + 1];
	char *point;
	kug_file *f = NULL;
	kug_status ret;
	int temp;
	kug_iterator *iter;
	int i;
	FILE *out;

	if(argc < 2)
		usage();

	if(strlen(argv[1]) > FILENAME_MAX)
		usage();
	strncpy(basedir, argv[1], FILENAME_MAX + 1);
	point = strrchr(basedir, '.');
	if(point != NULL)
		*point = '\0';

	ret = kug_open(argv[1], &f);
	if(ret != KUG_OK) {
		fprintf(stderr, "kug_open: %s\n", kug_strerror(ret));
		if(f != NULL)
			kug_free(f); /* this shouldn't crash, even with an error */
		exit(EXIT_FAILURE);
	}

	temp = mkdir(basedir, 0755);
	if(temp == 0) {
		iter = kug_init_iterator(f);
		if(iter != NULL) {
			while((i = kug_iter_next(iter)) != -1) {
				if(snprintf(filename, KUG_BUFFERSIZE, "%s/%s", basedir, f->item[i]->name) == KUG_BUFFERSIZE) {
					fprintf(stderr, "File entry too long.\n");
					exit(EXIT_FAILURE);
				}
				out = fopen(filename, "wb");
				if(out == NULL) {
					fprintf(stderr, "Couldn't open file %s.\n", filename);
					exit(EXIT_FAILURE);
				}
				ret = kug_copy(f, i, out);
				if(ret != KUG_OK) {
					fprintf(stderr, "kug_copy: %s\n", kug_strerror(ret));
					exit(EXIT_FAILURE);
				}
				fclose(out);

				fprintf(stderr, "%i/%i\r", i + 1, f->items);
			}
		}
		kug_free_iterator(iter);
	} else {
		perror("mkdir");
	}

	kug_free(f);

	exit(EXIT_SUCCESS);
}

void usage() {
	fprintf(stderr, "USAGE: exkug <filename>\n");
	exit(EXIT_FAILURE);
}
