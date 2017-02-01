#include "kugfile.h"

typedef struct {
	kug_file *f;
	unsigned int index;
} kug_iterator;

/*
 * Create an iterator for kug_items within a kug_file.  Due to implementation details, this is how you should fetch items.
 *
 * f		KUG context which contains the kug_items to iterate over.
 *
 * return	New kug iterator.
 */
kug_iterator *kug_init_iterator(kug_file *f);

/*
 * Free a kug_iterator.
 *
 * iter		Iterator to free.
 */
void kug_free_iterator(kug_iterator *iter);

/*
 * Get the next index.
 *
 * iter		Iterator to iterate.
 *
 * return	Next index.
 */
int kug_iter_next(kug_iterator *iter);

/*
 * Reset iterator.
 *
 * iter		Iterator to reset.
 */
void kug_iter_reset(kug_iterator *iter);
