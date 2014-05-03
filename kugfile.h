#include "kugerr.h"

#define KUG_BUFFERSIZE (1024 * 64)
#define KUG_ITEMLISTSIZE (5000)

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
} kug_file;

kug_file *kug_init();
void kug_free(kug_file *f);
kug_status kug_open(char *name, kug_file **f);
kug_status kug_open_dir(char *name, kug_file **f);
kug_status kug_close(kug_file *f);
kug_status kug_load(kug_file *f, int index);
void kug_unload(kug_file *f, int index);
kug_status kug_copy(kug_file *f, int index, FILE *out);
kug_status kug_write(kug_file *f, void (*statuscallback)(int, int));
