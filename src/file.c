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

Error File_close(File *self){
    if (self == NULL) {
        return ERROR(ERR_INVALID_ARGUMENT, "Invalid FIle");    
    }

    //already closed
    if(!self->is_open || self->handle == NULL){
        return ERROR_OK;
    }

    if (fclose(self->handle) != 0) {
        int saved_errno = errno;

        //even on failure, clear fields to avoid double-free/double-close
        self->handle = NULL;
        self->path=NULL;
        self->is_open=false;

        Error err = ERROR(ERR_FILE_CLOSE, "Could not close file");
        err.system_code = saved_errno;

        return err;
    }

    self->handle = NULL;
    self->path=NULL;
    self->is_open=false;

    return ERROR_OK;
}