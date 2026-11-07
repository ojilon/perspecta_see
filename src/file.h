#ifndef PERSPECTA_FILE_H
#define PERSPECTA_FILE_H

#include <stdio.h>
#include "error.h"

typedef struct {
    FILE *handle;
    const char *path;
    bool is_open;
}File;

Error File_open(File *self, const char *path, const char *mode);
Error File_close(File *self);

#endif