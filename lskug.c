#include <stdio.h>
#include <stdlib.h>

#include "kugfile.h"

int main(int argc, char **argv) {
	kug_file *f = NULL;
	kug_status ret;
	int i;

	if(argc < 2) {
		fprintf(stderr, "USAGE: lskug <filename>\n");
		exit(EXIT_FAILURE);
	}

	ret = kug_open(argv[1], &f);
	if(ret != KUG_OK) {
		fprintf(stderr, "kug_open: %s\n", kug_strerror(ret));
		if(f != NULL)
			kug_free(f); /* this shouldn't crash, even with an error */
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < f->items; i++)
		fprintf(stderr, "%i: %s @%li\n", i, f->item[i]->name, f->item[i]->itempos);

	kug_free(f);

	exit(EXIT_SUCCESS);
}
