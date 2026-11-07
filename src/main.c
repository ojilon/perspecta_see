#include "error.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAX_PATH_LENGTH 260

typedef struct{
    char file_path[MAX_PATH_LENGTH];
    int line_count;
    long file_size;
}FileInfo;


//-------------------------
//function declarations
//-----------------------------------
Error Unpack_for_file_properties(FileInfo *self, const char *path);
Error Count_lines(FileInfo *self);




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

    //count lines
    err = Count_lines(&Target_file);
    if(!error_is_ok(&err)) {
        error_print(&err);
        return 1;
    }

    printf("Path: %s\n", Target_file.file_path);
    printf("Size: %ld\n", Target_file.file_size);
    printf("line cout: %d\n", Target_file.line_count);
    
    return 0;

}


//---------------------------------
// FUNCTION IMPLEMENTATIONS
// ---------------------------------
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


//Count the number of lines in the file
Error Count_lines(FileInfo *self){
    Error err = ERROR_OK;
    FILE *file = NULL;

    int ch;
    int last = '\n'; /*an empty file counts as 0 lines */
    int lines = 0;
    long size = 0;

    //still an unclear error, should be able to later show what more
    // information about the error concerning what the function received
    if (self == NULL) {
        return ERROR(ERR_INVALID_ARGUMENT, "NULL FileInfo");    
    }

    file = fopen(self->file_path, "rb");
    if (file == NULL) {
        int saved = errno;
        err = ERROR(ERR_FILE_OPEN, "Coud not open file");
        err.system_code = saved;
        goto cleanup;    
    }

    //count the number of lines in the file
    while ((ch = fgetc(file)) != EOF) {
        size++;
        if(ch == '\n') lines++;
        last = ch;    
    }
    
    if (ferror(file)) {
        err = ERROR(ERR_FILE_READ, "Error while reading file");
        goto cleanup;    
    }

    if (last != '\n') lines++; /*last line had no trailing new line */
    
    /*only write results once everything succeeds */
    self->line_count = lines;
    self->file_size = size;    

    cleanup:
       if(file != NULL) fclose(file);
       return err;    
}