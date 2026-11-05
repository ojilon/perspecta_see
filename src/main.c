#include <stdio.h>
#include <string.h>

#define MAX_PATH_LENGTH 260

typedef struct{
    char file_path[MAX_PATH_LENGTH];
    int line_count;
    long file_size;
}FileInfo;

void Unpack_for_file_properties(FileInfo *self, const char *path) {
    strncpy(self->file_path, path, MAX_PATH_LENGTH-1);
    self->file_path[MAX_PATH_LENGTH-1] = '\0'; //null terminate
    self->line_count = 0;
    self->file_size = 0;
}


//-------------------------------------------------
// ENTRY POINT
// ------------------------------------------------

int main(int argc, char *argv[]) {
    //file object
    FileInfo Target_file;

    if(argc < 2) {
        printf("Usage: .\\perspecta.exe <path_to_file.c> ");
        return 1;
    }

    //pick file properties
    Unpack_for_file_properties(&Target_file, argv[1]);

    printf("Path to file: %s\n", Target_file.file_path);
    printf("Line count: %d\n", Target_file.line_count );
    
    return 0;

}
