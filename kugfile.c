#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kugfile.h"

#define __get_cur_item(KF) (KF->item[KF->items - 1])
#define __get_next_item(KF) (KF->item[KF->items])

static char __filebuffer[KUG_BUFFERSIZE];

kug_file *kug_init() {
	kug_file *f;

	f = malloc(sizeof(kug_file));
	if(f == NULL)
		goto error0;

	f->item = malloc(sizeof(kug_item *) * KUG_ITEMLISTSIZE);
	if(f->item == NULL)
		goto error1;

	f->itemlistsize = KUG_ITEMLISTSIZE;
	f->items = 0;
	f->file = NULL;
	f->basename = NULL;
	f->src = KUG_SRC_UNKNOWN;

	return(f);

error1:
	free(f);
error0:
	return(NULL);
}

static kug_item *__item_init() {
	kug_item *it;

	it = malloc(sizeof(kug_item));
	if(it == NULL)
		return(NULL);

	it->name = NULL;
	it->length = 0;
	it->data = NULL;
	it->itempos = 0;

	return(it);
}

static void __item_free(kug_item *it) {
	if(it->name != NULL)
		free(it->name);
	if(it->data != NULL)
		free(it->data);
	free(it);
}

kug_status kug_add_item(kug_file *f) {
	kug_item **temp;

	if(f->items == f->itemlistsize) {
		temp = realloc(f->item, sizeof(kug_item *) * f->itemlistsize * 2);
		if(temp == NULL)
			return(KUG_NOMEM);

		f->item = temp;
		f->itemlistsize *= 2;
	}

	__get_next_item(f) = __item_init();
	if(__get_next_item(f) == NULL)
		return(KUG_NOMEM);
	f->items++;

	return(KUG_OK);
}

static void __item_remove_last(kug_file *f) {
	__item_free(__get_cur_item(f));
	f->items--;
}

void kug_free(kug_file *f) {
	kug_close(f);
	while(f->items > 0)
		__item_remove_last(f);

	if(f->basename != NULL)
		free(f->basename);
	free(f->item);
	free(f);
}

static int __get_null_str(FILE *file, char *buf, size_t size) {
	long pos;
	unsigned int i;

	pos = ftell(file);
	if(fread(buf, 1, size, file) == 0)
		return 0;
	for(i = 0; i < size; i++) {
		if(buf[i] == '\0')
			break;
	}

	if(fseek(file, pos + i, SEEK_SET) < 0)
		return(-1);

	return(i);
}

static kug_status __get_name_mem(kug_file *f, size_t size) {
	kug_item *it = __get_cur_item(f);

	it->name = malloc(size);
	if(it->name == NULL)
		return(KUG_NOMEM);

	return(KUG_OK);
}

static kug_status __read_item(kug_file *f) {
	kug_status ret;
	kug_item *it;
	int temp;

	ret = kug_add_item(f);
	if(ret != KUG_OK)
		goto error0;
	it = __get_cur_item(f);

	it->itempos = ftell(f->file);
	if(it->itempos == -1) {
		ret = KUG_IO;
		goto error1;
	}

	temp = __get_null_str(f->file, __filebuffer, KUG_BUFFERSIZE);
	if(temp == KUG_BUFFERSIZE) {
		ret = KUG_BADFMT;
		goto error1;
	} else if(temp == 0 && feof(f->file)) { /* end of file reached, free the last entry and return EOF */
		ret = KUG_EOF;
		goto error1;
	}
	temp++; /* add null terminator which will have been read */

	ret = __get_name_mem(f, temp);
	if(ret != KUG_OK)
		goto error1;
	memcpy(it->name, __filebuffer, temp);

	if(fseek(f->file, 1, SEEK_CUR) == -1) { /* skip null terminator */
		ret = KUG_IO;
		goto error1;
	}

	temp = fread(&(it->length), 1, sizeof(int), f->file);
	if(temp < 0) {
		ret = KUG_IO;
		goto error1;
	} else if(temp < (int)sizeof(int)) {
		ret = KUG_BADFMT;
		goto error1;
	}

	return(KUG_OK);

error1:
	__item_remove_last(f);
error0:
	return(ret);
}

static kug_status __read_items(kug_file *f) {
	kug_status ret;

	for(;;) {
		ret = __read_item(f);
		if(ret == KUG_EOF)
			break;
		else if(ret != KUG_OK)
			return(ret);
		if(fseek(f->file, (long)(__get_cur_item(f)->length), SEEK_CUR) < 0)
			return(KUG_IO);
	}

	return(KUG_OK);
}

kug_status kug_open(char *name, kug_file **fp) {
	kug_file *f;
	kug_status ret = KUG_ERROR;
	char *point;

	f = kug_init();
	if(f == NULL) {
		ret = KUG_NOMEM;
		goto error0;
	}

	f->file = fopen(name, "rb");
	if(f->file == NULL) {
		ret = KUG_NOENT;
		goto error1;
	}

	f->basename = malloc(strlen(name) + 1);
	if(f->basename == NULL) {
		ret = KUG_NOMEM;
		goto error1;
	}
	strcpy(f->basename, name);
	point = strrchr(f->basename, '.');
	if(point != NULL)
		*point = '\0';

	ret = __read_items(f);
	if(ret != KUG_OK)
		goto error1;

	f->src = KUG_SRC_FILE;

	*fp = f;
	return(KUG_OK);

error1:
	kug_free(f);
error0:
	return(ret);
}

static int __all_loaded(kug_file *f) {
	int i;

	for(i = 0; i < f->items; i++) {
		if(f->item[i] != NULL && f->item[i]->data == NULL)
			return(0);
	}

	return(1);
}

kug_status kug_open_for_write(char *name, kug_file *f) {
	kug_status ret = KUG_ERROR;
	char *point;

	if(f->src == KUG_SRC_FILE && !__all_loaded(f)) {
		ret = KUG_BADFMT;
		goto error0;
	}

	ret = kug_close(f);
	if(ret != KUG_OK)
		goto error0;

	f->file = fopen(name, "wb");
	if(f->file == NULL) {
		ret = KUG_NOENT;
		goto error1;
	}

	if(f->basename != NULL)
		free(f->basename);
	f->basename = malloc(strlen(name) + 1);
	if(f->basename == NULL) {
		ret = KUG_NOMEM;
		goto error1;
	}
	strcpy(f->basename, name);
	point = strrchr(f->basename, '.');
	if(point != NULL)
		*point = '\0';

	return(KUG_OK);

error1:
	kug_free(f);
error0:
	return(ret);
}

static kug_status __read_dir_item(kug_file *f, struct dirent *de) {
	kug_status ret = KUG_ERROR;
	struct stat st;
	kug_item *it;
	int len;
	char filename[KUG_BUFFERSIZE];

	len = snprintf(filename, KUG_BUFFERSIZE, "%s/%s", f->basename, de->d_name);
	if(len < 0) {
		ret = KUG_IO;
		goto error0;
	} else if(len == KUG_BUFFERSIZE) {
		ret = KUG_BADFMT;
		goto error0;
	}

	if(stat(filename, &st) < 0) {
		ret = KUG_IO;
		goto error0;
	}
	if(!S_ISREG(st.st_mode)) { /* don't do anything if it's not a regular file, this isn't an error */
		ret = KUG_OK;
		goto error0;
	}

	ret = kug_add_item(f);
	if(ret != KUG_OK)
		goto error0;
	it = __get_cur_item(f);

	ret = __get_name_mem(f, strlen(de->d_name) + 1);
	if(ret != KUG_OK)
		goto error1;
	strcpy(it->name, de->d_name);

	if(st.st_size > 0xFFFFFFFF) {
		ret = KUG_BADFMT;
		goto error1;
	}
	it->length = (unsigned int)st.st_size;

	return(KUG_OK);

error1:
	__item_remove_last(f);
error0:
	return(ret);
}

static kug_status __read_dir_items(kug_file *f) {
	kug_status ret = KUG_ERROR;
	DIR *dir;
	struct dirent *de;

	dir = opendir(f->basename);
	if(dir == NULL) {
		ret = KUG_IO;
		goto error0;
	}

	for (;;) {
		de = readdir(dir);
		if(de == NULL)
			break;

		ret = __read_dir_item(f, de);
		if(ret != KUG_OK)
			goto error1;
	}

	return(KUG_OK);

error1:
	closedir(dir);
error0:
	return(ret);
}

kug_status kug_open_dir(char *name, kug_file **fp) {
	kug_file *f;
	kug_status ret = KUG_ERROR;

	f = kug_init();
	if(f == NULL) {
		ret = KUG_NOMEM;
		goto error0;
	}

	f->basename = malloc(strlen(name) + 1);
	if(f->basename == NULL) {
		ret = KUG_NOMEM;
		goto error1;
	}
	strcpy(f->basename, name);

	ret = __read_dir_items(f);
	if(ret != KUG_OK)
		goto error1;

	f->src = KUG_SRC_DIRECTORY;

	*fp = f;
	return(KUG_OK);

error1:
	kug_free(f);
error0:
	return ret;
}

kug_status kug_close(kug_file *f) {
	if(f->file == NULL) /* already closed so no effect */
		return(KUG_OK);

	if(fclose(f->file) < 0)
		return(KUG_IO);
	f->file = NULL;

	if(f->basename == NULL)
		return(KUG_BADFMT);
	free(f->basename);
	f->basename = NULL;
	f->src = KUG_SRC_UNKNOWN;

	return(KUG_OK);
}

static kug_status __item_seekto(kug_file *f, int index) {
	if(fseek(f->file, f->item[index]->itempos, SEEK_SET) < 0)
		return(KUG_IO);

	if(__get_null_str(f->file, __filebuffer, KUG_BUFFERSIZE) == KUG_BUFFERSIZE)
		return(KUG_BADFMT);
	
	if(fseek(f->file, 5, SEEK_CUR) < 0) /* skup null terminator and length */
		return(KUG_IO);

	return(KUG_OK);
}

kug_status kug_load(kug_file *f, int index) {
	kug_status ret;
	char filename[FILENAME_MAX];
	FILE *in = NULL;
	int length;

	if(f->item[index] == NULL) /* Empty item, nothing to do. */
		return(KUG_OK);

	if(f->src == KUG_SRC_FILE) {
		f->item[index]->data = malloc(f->item[index]->length);
		if(f->item[index]->data == NULL) {
			ret = KUG_NOMEM;
			goto error0;
		}

		ret = __item_seekto(f, index);
		if(ret != KUG_OK)
			goto error2;

		if(fread(f->item[index]->data, 1, f->item[index]->length, f->file) < f->item[index]->length) {
			ret = KUG_IO;
			goto error2;
		}
	} else if(f->src == KUG_SRC_DIRECTORY) {
		if(snprintf(filename, FILENAME_MAX, "%s/%s", f->basename, f->item[index]->name) == FILENAME_MAX) {
			ret = KUG_BADFMT;
			goto error0;
		}

		in = fopen(filename, "rb");
		if(in == NULL) {
			ret = KUG_IO;
			goto error0;
		}

		if(fseek(in, SEEK_END, 0) < 0) {
			ret = KUG_IO;
			goto error1;
		}
		length = ftell(in);
		if(length < 0) {
			ret = KUG_IO;
			goto error1;
		}
		f->item[index]->length = length;
		if(fseek(in, SEEK_SET, 0) < 0) {
			ret = KUG_IO;
			goto error1;
		}

		f->item[index]->data = malloc(f->item[index]->length);
		if(f->item[index]->data == NULL) {
			ret = KUG_NOMEM;
			goto error1;
		}

		if(fread(f->item[index]->data, 1, f->item[index]->length, f->file) < f->item[index]->length) {
			ret = KUG_IO;
			goto error2;
		}
	} else {
		return(KUG_BADFMT);
	}

	return(KUG_OK);

error2:
	free(f->item[index]->data);
	f->item[index]->data = NULL;
error1:
	if(in != NULL)
		fclose(in);
error0:
	return(ret);
}

kug_status kug_load_all(kug_file *f, void (*statuscallback)(int, int)) {
	int i;
	kug_status ret;

	for(i = 0; i < f->items; i++) {
		ret = kug_load(f, i);
		if(ret != KUG_OK)
			return(ret);
		if(statuscallback != NULL)
			statuscallback(i + 1, f->items);
	}

	return(KUG_OK);
}

FILE *__open_file_index(kug_file *f, int index, const char *mode) {
	char filename[KUG_BUFFERSIZE];
	FILE *in;
	int len;

	len = snprintf(filename, KUG_BUFFERSIZE, "%s/%s", f->basename, f->item[index]->name);
	if(len < 0 || len == KUG_BUFFERSIZE)
		return(NULL);

	in = fopen(filename, mode);
	if(in == NULL)
		return(NULL);

	return(in);
}

void kug_unload(kug_file *f, int index) {
	if(f->item[index] != NULL) {
		if(f->item[index]->data == NULL)
			return;
		free(f->item[index]->data);
		f->item[index]->data = NULL;
	}
}

kug_status __copy_data_out(kug_file *f, int index, FILE *out) {
	unsigned int i;
	unsigned int len;
	kug_status ret;

	for(i = 0; i < f->item[index]->length; i += KUG_BUFFERSIZE) {
		ret = __item_seekto(f, index);
		if(ret != KUG_OK)
			return(ret);

		len = KUG_BUFFERSIZE > f->item[index]->length - i ? f->item[index]->length : KUG_BUFFERSIZE;
		if(fread(__filebuffer, 1, len, f->file) < len)
			return(KUG_IO);
		if(fwrite(__filebuffer, 1, len, out) < len)
			return(KUG_IO);
	}

	return(KUG_OK);
}

kug_status __copy_data_in(kug_file *f, FILE *in) {
	unsigned int len, totlen;
	long cur, end;

	cur = ftell(f->file);
	if(cur < 0)
		return(KUG_IO);
	totlen = 0;
	if(fwrite(&totlen, 1, sizeof(int), f->file) < sizeof(int))
		return(KUG_IO);

	for(;;) {
		len = fread(__filebuffer, 1, KUG_BUFFERSIZE, in);
		if(len < KUG_BUFFERSIZE) {
			if(feof(in)) {
				if(fwrite(__filebuffer, 1, len, f->file) < len)
					return(KUG_IO);
				totlen += len;
				break;
			} else {
				return(KUG_IO);
			}
		}

		if(fwrite(__filebuffer, 1, len, f->file) < len)
			return(KUG_IO);
		totlen += len;
	}

	end = ftell(f->file);
	if(end < 0)
		return(KUG_IO);
	if(fseek(f->file, cur, SEEK_SET) < 0)
		return(KUG_IO);
	if(fwrite(&totlen, 1, sizeof(int), f->file) < sizeof(int))
		return(KUG_IO);
	if(fseek(f->file, end, SEEK_SET) < 0)
		return(KUG_IO);

	return(KUG_OK);
}

kug_status kug_copy(kug_file *f, int index, FILE *out) {
	kug_status ret;

	if(f->item[index] == NULL)
		return(KUG_NOENT);

	if(f->item[index]->data != NULL) {
		if(fwrite(f->item[index]->data, 1, f->item[index]->length, out) < f->item[index]->length)
			return(KUG_IO);
	}

	if(f->file == NULL)
		return(KUG_IO);

	ret = __copy_data_out(f, index, out);
	if(ret != KUG_OK)
		return(ret);

	return(KUG_OK);
}

static kug_status __write_item(kug_file *f, int index) {
	kug_status ret;
	FILE *in;

	if(fprintf(f->file, "%s", f->item[index]->name) < (int)strlen(f->item[index]->name))
		return(KUG_IO);
	if(fputc(0, f->file) == EOF)
		return(KUG_IO);
	if(f->item[index]->data != NULL) { /* if the data is already in memory, write it */
		if(fwrite(&(f->item[index]->length), 1, sizeof(int), f->file) < sizeof(int)) /* write size */
			return(KUG_IO);
		if(fwrite(f->item[index]->data, 1, f->item[index]->length, f->file) < f->item[index]->length)
			return(KUG_IO);
	} else {
		in = __open_file_index(f, index, "rb");
		if(in == NULL)
			return(KUG_IO);
		ret = __copy_data_in(f, in);
		fclose(in);

		if(ret != KUG_OK)
			return(ret);
	}

	return(KUG_OK);
}

kug_status kug_write(kug_file *f, void (*statuscallback)(int, int)) {
	int i;
	kug_status ret;

	if(f->src != KUG_SRC_DIRECTORY && !__all_loaded(f))
		return(KUG_BADFMT);

	for(i = 0; i < f->items; i++) {
		if(f->item[i] != NULL) {
			ret = __write_item(f, i);
			if(ret != KUG_OK)
				return(ret);
			if(statuscallback != NULL)
				statuscallback(i + 1, f->items);
		}
	}

	return(KUG_OK);
}

kug_status kug_del_item(kug_file *f, int index) {
	if(f->item[index] == NULL)
		return(KUG_NOENT);

	__item_free(f->item[index]);
	f->item[index] = NULL;

	if(index + 1 == f->items)
		f->items--;

	return(KUG_OK);
}
