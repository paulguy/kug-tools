typedef enum {
	KUG_OK = 0,	/* Success */
	KUG_ERROR,	/* Undefined error */
	KUG_NOMEM,	/* Couldn't allocate memory */
	KUG_NOENT,	/* Couldn't open file */
	KUG_BADFMT,	/* Bad/corrupt file format */
	KUG_IO,		/* Data access failed */
	KUG_EOF,	/* End of file reached */

	KUG_ERRMAX	/* Last entry */
} kug_status;

const char *kug_strerror(kug_status e);
