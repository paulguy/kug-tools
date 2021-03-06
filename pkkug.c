#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kugfile.h"

void printstatus(void *data, unsigned int complete, unsigned int total);

int main(int argc, char **argv) {
	kug_file *f = NULL;
	kug_status ret;
	char *filename;
	int namelen;

	if(argc < 2) {
		fprintf(stderr, "USAGE: pkkug <dirname>\n");
		exit(EXIT_FAILURE);
	}

	namelen = strlen(argv[1]);
	filename = malloc(namelen + 4 + 1); /* length + ".bin" + '\0' */
	strcpy(filename, argv[1]);
	if(filename[namelen - 1] == '/') /* remove trailing slash, if any */
		namelen--;
	strcpy(&filename[namelen], ".bin");

	ret = kug_open_dir(argv[1], &f);
	if(ret != KUG_OK) {
		fprintf(stderr, "kug_open_dir: %s\n", kug_strerror(ret));
		if(f != NULL)
			kug_free(f); /* this shouldn't crash, even with an error */
		exit(EXIT_FAILURE);
	}

	ret = kug_open_for_write(filename, f);
	if(f->file == NULL) {
		fprintf(stderr, "kug_open_for_write: %s: %s.", filename, kug_strerror(ret));
	} else {
		ret = kug_write(f, NULL, printstatus);
		fprintf(stderr, "\n");
		if(ret != KUG_OK) {
			fprintf(stderr, "kug_write: %s\n", kug_strerror(ret));
			kug_free(f);
			exit(EXIT_FAILURE);
		}
	}

	kug_free(f);

	exit(EXIT_SUCCESS);
}

void printstatus(void *data, unsigned int complete, unsigned int total) {
	fprintf(stderr, "%i/%i\r", complete, total);
}
