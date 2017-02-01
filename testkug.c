#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kugfile.h"
#include "kugiter.h"

#define TEST_NAME "test.bin"
#define TEST_OUT_NAME "testout.bin"
#define TEST_BASENAME "test"
#define TEST_ITEMS (5)
#define TEST_CLEANUP() test_file = NULL; test_iterator = NULL;
#define TEST_PASS() {line = __LINE__; TEST_CLEANUP() return(0);}
#define TEST_FAIL() {line = __LINE__; return(1);}
#define TEST_ERROR() {line = __LINE__; return(-1);}

const char *test_failed = "FAILED!";
const char *test_passed = "PASSED!";
const char *test_error = "!ERROR!";
int line;

kug_file *test_file;
kug_item *test_item;
kug_iterator *test_iterator;

const char *run_test(int (testfunc)(void));
void statustest(void *count, unsigned int cur, unsigned int total);

/* initial function tests */
int test_kug_init();
int test_kug_free();
int test_kug_open();
int test_kug_copy();
int test_kug_close();
int test_kug_open_dir();
int test_kug_open_for_write();
int test_kug_write();
int test_kug_load();
int test_kug_unload();
int test_kug_load_all();
int test_kug_add_item();
int test_kug_del_item();

int main(int argc, char **argv) {
	printf("KUG files test suite\n"
	        "This program will try to report passes or fails, but some possible crash checks are attempted; these are failures as well!\n\n");
	printf("Initial Function Tests\n");
	printf("kug_init(): %s @%i\n", run_test(test_kug_init), line);
	printf("kug_free(): %s @%i\n", run_test(test_kug_free), line);
	printf("kug_open(): %s @%i\n", run_test(test_kug_open), line);
	printf("kug_copy(): %s @%i\n", run_test(test_kug_copy), line);
	printf("kug_close(): %s @%i\n", run_test(test_kug_close), line);
	printf("kug_open_dir(): %s @%i\n", run_test(test_kug_open_dir), line);
	printf("kug_open_for_write(): %s @%i\n", run_test(test_kug_open_for_write), line);
	printf("kug_write(): %s @%i\n", run_test(test_kug_write), line);
	printf("kug_load(): %s @%i\n", run_test(test_kug_load), line);
	printf("kug_unload(): %s @%i\n", run_test(test_kug_unload), line);
	printf("kug_load_all(): %s @%i\n", run_test(test_kug_load_all), line);
	printf("kug_add_item(): %s @%i\n", run_test(test_kug_add_item), line);
	printf("kug_del_item(): %s @%i\n", run_test(test_kug_del_item), line);
}

const char *run_test(int (testfunc)(void)) {
	int ret;

	test_file = NULL;
	test_iterator = NULL;

	ret = testfunc();

	/* we call the free functions here even if we don't trust them */
	if(test_file != NULL) kug_free(test_file);
	if(test_iterator != NULL) kug_free_iterator(test_iterator);

	if(ret < 0)
		return(test_error);
	else if(ret > 0)
		return(test_failed);

	return(test_passed);
}

void statustest(void *count, unsigned int cur, unsigned int total) {
	int *icount = (int *)count;

	if(*icount >= 0)
		(*icount)++;

	if(cur > total || (int)cur != *icount)
		*icount = -1;
}

int test_kug_init() {
	unsigned int i;

	test_file = kug_init();
	if(test_file == NULL) TEST_FAIL()

	if(test_file->items != 0) TEST_FAIL()
	if(test_file->basename != NULL) TEST_FAIL()
	if(test_file->itemlistsize < 1) TEST_FAIL()
	if(test_file->file != NULL) TEST_FAIL()

	for(i = 0; i < test_file->itemlistsize; i++)
		test_item = test_file->item[i];

	kug_free(test_file); /* we may not trust this yet, but we don't want to potentially leak memory */

	TEST_PASS()
}

int test_kug_free() {
	test_file = kug_init();

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_open() {
	kug_status ret;
	unsigned int i;

	ret = kug_open(TEST_NAME, &test_file);
	if(ret != KUG_OK) TEST_FAIL()

	if(strcmp(test_file->basename, TEST_BASENAME) != 0) TEST_FAIL()
	if(test_file->items != TEST_ITEMS) TEST_FAIL()
	if(test_file->src != KUG_SRC_FILE) TEST_FAIL()
	for(i = 0; i < test_file->items; i++) {
		if(test_file->item[i]->data != NULL) TEST_FAIL()
	}

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_copy() {
	kug_status ret;
	unsigned int i;
	FILE *out;

	ret = kug_open(TEST_NAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	for(i = 0; i < test_file->items; i++) {
		out = fopen(TEST_OUT_NAME, "wb");
		if(out == NULL) TEST_ERROR()
		ret = kug_copy(test_file, i, out);
		fclose(out);
		if(ret != KUG_OK) TEST_FAIL()
	}

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_close() {
	kug_status ret;

	ret = kug_open(TEST_NAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_close(test_file);
	if(ret != KUG_OK) TEST_FAIL()

	if(test_file->src != KUG_SRC_UNKNOWN) TEST_FAIL()
	if(test_file->file != NULL) TEST_FAIL()

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_open_dir() {
	kug_status ret;
	unsigned int i;

	ret = kug_open_dir(TEST_BASENAME, &test_file);
	if(ret != KUG_OK) TEST_FAIL()

	if(strcmp(test_file->basename, TEST_BASENAME) != 0) TEST_FAIL()
	if(test_file->items != TEST_ITEMS) TEST_FAIL()
	if(test_file->src != KUG_SRC_DIRECTORY) TEST_FAIL()
	for(i = 0; i < test_file->items; i++) {
		if(test_file->item[i]->data != NULL) TEST_FAIL()
	}

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_open_for_write() {
	kug_status ret;

	ret = kug_open_dir(TEST_BASENAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_open_for_write(TEST_OUT_NAME, test_file);
	if(ret != KUG_OK) TEST_FAIL()

	if(test_file->file == NULL) TEST_FAIL()
	if(test_file->src == KUG_SRC_FILE) TEST_FAIL()

	kug_close(test_file);
	kug_free(test_file);

	TEST_PASS()
}

int test_kug_write() {
	kug_status ret;
	int count;

	ret = kug_open_dir(TEST_BASENAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_open_for_write(TEST_OUT_NAME, test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_write(test_file, &count, statustest);
	if(ret != KUG_OK) TEST_FAIL()

	if(count != (int)test_file->items) TEST_FAIL()

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_load() {
	kug_status ret;
	unsigned int i, j;
	volatile char testchr = 0;

	ret = kug_open(TEST_NAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	for(i = 0; i < test_file->items; i++) {
		ret = kug_load(test_file, i);
		if(ret != KUG_OK) TEST_FAIL()
	}

	for(i = 0; i < test_file->items; i++) {
		if(test_file->item[i] == NULL) TEST_FAIL()
		for(j = 0; j < test_file->item[i]->length; j++)
			testchr += test_file->item[i]->data[j];
	}

	kug_free(test_file);

	ret = kug_open_dir(TEST_BASENAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	for(i = 0; i < test_file->items; i++) {
		ret = kug_load(test_file, i);
		if(ret != KUG_OK) TEST_FAIL()
	}

	for(i = 0; i < test_file->items; i++) {
		if(test_file->item[i] == NULL) TEST_FAIL()
		for(j = 0; j < test_file->item[i]->length; j++)
			testchr += test_file->item[i]->data[j];
	}

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_unload() {
	kug_status ret;

	ret = kug_open(TEST_NAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_load(test_file, 0);
	if(ret != KUG_OK) TEST_ERROR()
	if(test_file->item[0]->data == NULL) TEST_ERROR()

	kug_unload(test_file, 0);
	if(test_file->item[0]->data != NULL) TEST_FAIL()

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_load_all() {
	kug_status ret;
	unsigned int i, j;
	volatile char testchr = 0;
	int count;

	ret = kug_open(TEST_NAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_load_all(test_file, &count, statustest);

	if(count != (int)test_file->items) TEST_FAIL()

	for(i = 0; i < test_file->items; i++) {
		if(test_file->item[i] == NULL) TEST_FAIL()
		for(j = 0; j < test_file->item[i]->length; j++)
			testchr += test_file->item[i]->data[j];
	}

	kug_free(test_file);
	count = 0;

	ret = kug_open_dir(TEST_BASENAME, &test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_load_all(test_file, &count, statustest);

	if(count != (int)test_file->items) TEST_FAIL()

	for(i = 0; i < test_file->items; i++) {
		if(test_file->item[i] == NULL) TEST_FAIL()
		for(j = 0; j < test_file->item[i]->length; j++)
			testchr += test_file->item[i]->data[j];
	}

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_add_item() {
	kug_status ret;

	test_file = kug_init();
	if(test_file == NULL) TEST_ERROR()

	ret = kug_add_item(test_file);
	if(ret != KUG_OK) TEST_FAIL()

	if(test_file->items != 1) TEST_FAIL()

	kug_free(test_file);

	TEST_PASS()
}

int test_kug_del_item() {
	kug_status ret;

	test_file = kug_init();
	if(test_file == NULL) TEST_ERROR()

	ret = kug_add_item(test_file);
	if(ret != KUG_OK) TEST_ERROR()

	ret = kug_del_item(test_file, 0);
	if(ret != KUG_OK) TEST_FAIL()

	if(test_file->items != 0) TEST_FAIL()

	kug_free(test_file);

	TEST_PASS()
}
