#include "kugerr.h"

const char *KUG_ERRSTR[KUG_ERRMAX] = {
	"Success",
	"Undefined error",
	"Couldn't allocate memory",
	"Couldn't open file",
	"Bad/corrupt file format",
	"Data access failed",
	"End of file reached"
};

const char *kug_strerror(kug_status e) {
	return(KUG_ERRSTR[e]);
}

