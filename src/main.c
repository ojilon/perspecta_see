#include "error.h"
#include <stdio.h>
#include <string.h>

#define MAX_PATH_LENGTH 260

typedef struct{
    char file_path[MAX_PATH_LENGTH];
    int line_count;
    long file_size;
}FileInfo;

Error Unpack_for_file_properties(FileInfo *self, const char *path) {

    if(self == NULL || path == NULL){
        return ERROR(ERR_INVALID_ARGUMENT, "NULL argument");
    }

    if (strlen(path) >= MAX_PATH_LENGTH) {
        return ERROR(ERR_INVALID_ARGUMENT,"Path is too long");    
    }
    
    strcpy(self->file_path, path);
    self->line_count = 0;
    self->file_size = 0;

    return ERROR_OK;
}


//-------------------------------------------------
// ENTRY POINT
// ------------------------------------------------

int main(int argc, char *argv[]) {
    //file object
    FileInfo Target_file;

    if(argc < 2) {
        fprintf(stderr, "Usage: .\\perspecta.exe <path_to_file.c>\n");
        return 1;
    }

    //pick file properties
    Error err = Unpack_for_file_properties(&Target_file, argv[1]);
    if (!error_is_ok(&err)) {
        error_print(&err);
        return 1;    
    }

    printf("Path to file: %s\n", Target_file.file_path);
    printf("Line count: %d\n", Target_file.line_count );
    
    return 0;

}
