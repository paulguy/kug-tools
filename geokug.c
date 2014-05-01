#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "kugfile.h"

typedef struct {
	int x, y;
} position;

void parsename(position *pos, char *name);
int containspos(kug_file *f, position *pos);

int main(int argc, char **argv) {
	kug_file *f = NULL;
	kug_status ret;
	char *filename;
	position pos, min, max;
	int i, j;

	min.x = INT_MAX;
	min.y = INT_MAX;
	max.x = INT_MIN;
	max.y = INT_MIN;

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

	for(i = 0; i < f->items; i++) {
		parsename(&pos, f->item[i]->name);

		if(pos.x < min.x)
			min.x = pos.x;
		if(pos.x > max.x)
			max.x = pos.x;
		if(pos.y < min.y)
			min.y = pos.y;
		if(pos.y > max.y)
			max.y = pos.y;
	}

	fprintf(stderr, "Dimensions:  X: %i - %i  Y: %i - %i\n", min.x, max.x, min.y, max.y);

	for(i = min.y; i < max.y; i++) {
		for(j = min.x; j < max.x; j++) {
			pos.x = i;
			pos.y = j;
			if(containspos(f, &pos))
				fputc('#', stderr);
			else
				fputc(' ', stderr);
		}
		fputc('\n', stderr);
	}

	kug_free(f);

	exit(EXIT_SUCCESS);
}

void parsename(position *pos, char *name) {
	char xstr[KUG_BUFFERSIZE];
	char *ystr, *temp;

	strcpy(xstr, name);
	temp = strchr(xstr, ' ');
	*temp = '\0';
	ystr = strchr(xstr, ',');
	*ystr = '\0';
	ystr++;

	pos->x = atoi(xstr);
	pos->y = atoi(ystr);
}

int containspos(kug_file *f, position *pos) {
	int i;
	position cur;

	for(i = 0; i < f->items; i++) {
		parsename(&cur, f->item[i]->name);
		if(memcmp(pos, &cur, sizeof(position)) == 0)
			return(1);
	}

	return(0);
}
