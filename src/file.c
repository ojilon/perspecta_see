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

    if(!self->is_open){
        return ERROR_OK;
    }

    if (fclose(self->handle) != 0) {
        self->handle = NULL;
        self->path=NULL;
        self->is_open=false;

        return ERROR(ERR_FILE_CLOSE, "Could not close file");
    }

    self->handle = NULL;
    self->path=NULL;
    self->is_open=false;

    return ERROR_OK;
}