#include <stdio.h>

#define MAX_PATH_LENGTH 260

typedef struct{
    char file_path[MAX_PATH_LENGTH];
    int line_count;
}FileInfo;

int main(int argc, char *argv[]) {
    //file object
    FileInfo FileObject = {0};

    if(argc < 2) {
        printf("Usage: .\\perspecta.exe <path_to_file.c> ");
        return 1;
    }

    printf("Path to file: %s\n", argv[1]);
    return 0;

}
