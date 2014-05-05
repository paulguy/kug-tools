#include <stdio.h>
#include <stdlib.h>

#include "kugiter.h"

kug_iterator *kug_init_iterator(kug_file *f) {
	kug_iterator *iter;

	iter = malloc(sizeof(kug_iterator));
	if(iter == NULL)
		return(NULL);

	iter->f = f;
	iter->index = -1;

	return(iter);
}

void kug_free_iterator(kug_iterator *iter) {
	free(iter);
}

int kug_iter_next(kug_iterator *iter) {
	iter->index++;

	for(;;iter->index++) {
		if(iter->index >= iter->f->items)
			return(-1);

		if(iter->f->item[iter->index] != NULL)
			break;
	}

	return(iter->index);
}
