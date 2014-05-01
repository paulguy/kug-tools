#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kugfile.h"

int main(int argc, char **argv) {
	kug_file *f = NULL;
	kug_status ret;
	char *filename;

	if(argc < 2) {
		fprintf(stderr, "USAGE: pkkug <dirname>\n");
		exit(EXIT_FAILURE);
	}

	filename = malloc(strlen(argv[1]) + 4 + 1); /* length + ".bin" + '\0' */
	sprintf(filename, "%s.bin", argv[1]);

	ret = kug_open_dir(argv[1], &f);
	if(ret != KUG_OK) {
		fprintf(stderr, "kug_open_dir: %s\n", kug_strerror(ret));
		if(f != NULL)
			kug_free(f); /* this shouldn't crash, even with an error */
		exit(EXIT_FAILURE);
	}

	f->file = fopen(filename, "wb");
	if(f->file == NULL) {
		fprintf(stderr, "Failed to open file %s.", filename);
	} else {
		ret = kug_write(f);
		if(ret != KUG_OK) {
			fprintf(stderr, "kug_write: %s\n", kug_strerror(ret));
			kug_free(f);
			exit(EXIT_FAILURE);
		}
	}

	kug_free(f);

	exit(EXIT_SUCCESS);
}
