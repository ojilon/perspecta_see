#include "file.h"
#include "error.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

Error File_open(File *self, const char *path, const char *mode){
    if(self == NULL || path == NULL || mode == NULL) {
        return ERROR(ERR_INVALID_ARGUMENT, "invalide arguments");
    }

    //make sure is empty
    self->handle = NULL;
    self->path = NULL;
    self->is_open = false;

    FILE *h = fopen(path, mode);
    if(h == NULL) {
        int saved = errno;
        Error err = ERROR(ERR_FILE_OPEN, "could not open file");
        err.system_code = saved;
        return err;
    }

    self->handle = h;
    self->path = path;
    self->is_open = true;

    return  ERROR_OK;
}