#ifndef __KUGFILE_H
#define __KUGFILE_H

#include "kugerr.h"

#define KUG_BUFFERSIZE (1024 * 64)
#define KUG_ITEMLISTSIZE (5000)

typedef enum {
	KUG_SRC_UNKNOWN,
	KUG_SRC_FILE,
	KUG_SRC_DIRECTORY,
} kug_source;

typedef struct {
	char *name;
	unsigned int length;
	char *data;
	long itempos;
} kug_item;

typedef struct {
	char *basename;
	kug_item **item;
	int items;
	int itemlistsize;
	FILE *file;
	kug_source src;
} kug_file;

/*
 * Initialize a blank KUG data context.
 * 
 * return		New blank KUG context with no items and no source.
 */
kug_file *kug_init();

/*
 * Free a KUG data context and any open resources including files.
 *
 * f			KUG context to free.
 */
void kug_free(kug_file *f);

/*
 * Opens a KUG data context from a file and create indices.
 *
 * name			Filename to open.
 * f			Pointer to memory for new context to reside.
 *
 * return		Status.
 */
kug_status kug_open(char *name, kug_file **f);

/*
 * Open a new file for writing.  This will only work if the source is a directory or all data is in memory, else an error is returned.  No further processing is performed.
 *
 * name			Filename to open.
 * f			KUG context to operate on.
 *
 * return		Status.
 */
kug_status kug_open_for_write(char *name, kug_file *f);

/*
 * Opens a KUG data context from a directory and create indices.
 *
 * name			Name of directory to open.
 * f			Pointer to memory for new context to reside.
 *
 * return		Status.
 */
kug_status kug_open_dir(char *name, kug_file **f);

/*
 * Close open file and clears source.
 *
 * f			KUG context to operate on.
 *
 * return		Status.
 */
kug_status kug_close(kug_file *f);

/*
 * Load data in to memory.
 *
 * f			KUG context to operate on.
 * index		Index to load data in to.
 *
 * return		Status.
 */
kug_status kug_load(kug_file *f, int index);

/*
 * Load all data in to memory.
 *
 * f				KUG context to operate on.
 * statuscallback	Function called on completion of each item loaded, receives the last item loaded and total items to load.  Can be NULL to not call anything.
 *
 * return			Status.
 */
kug_status kug_load_all(kug_file *f, void (*statuscallback)(int, int));

/*
 * Unload data from memory.
 *
 * f			KUG context to operate on.
 * index		Item to unload.
 */
void kug_unload(kug_file *f, int index);

/*
 * Copy data from a file source or memory to a file, but not from a directory source.
 *
 * f			KUG context to operate on.
 * index		Index to get data from.
 * out			File to write data to.
 *
 * return		Status.
 */
kug_status kug_copy(kug_file *f, int index, FILE *out);

/*
 * Copy data from memory or directory source to a KUG file.  This will only work if the source is a directory or all data is in memory, else an error is returned.
 *
 * f				KUG context to operate on.
 * statuscallback	Function called on completion of each item written, receives the last item written and total items to write.  Can be NULL to not call anything.
 *
 * return			Status.
 */
kug_status kug_write(kug_file *f, void (*statuscallback)(int, int));

/*
 * Add a new item.
 *
 * f			KUG context to add to.
 *
 * return		Status.
 */
kug_status kug_add_item(kug_file *f);

/*
 * Deletes an item.
 *
 * f			KUG context to remove item from.
 * index		Item index to remove.
 *
 * return		Status.
 */
kug_status kug_del_item(kug_file *f, int index);

#endif
