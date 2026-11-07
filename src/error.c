#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>

Error error_make(ErrorCode code, const char *message, const char *file, int line){
    Error err;
    err.code = code;
    err.message = message;
    err.system_code = 0;
    err.file = file;
    err.line =  line;

    return err;
}

bool error_is_ok(const Error *err){
    return err->code == ERR_NONE ;
}

const char *error_name(ErrorCode code) {
    switch (code) {
        case ERR_NONE:             return "ERR_NONE";
        case ERR_INVALID_ARGUMENT: return "ERR_INVALID_ARGUMENT";
        case ERR_FILE_OPEN:        return "ERR_FILE_OPEN";
        case ERR_FILE_READ:        return "ERR_FILE_READ";
        case ERR_INTERNAL:         return "ERR_INTERNAL";
        case ERR_FILE_CLOSE:       return "ERR_FILE_CLOSE";
    }

    return "ERR_UNKNOWN";
}

void error_print(const Error *err){
    fprintf(stderr, "error [%s]: %s (%s:%d)\n", error_name(err->code), err->message, err->file, err->line);

    if (err->system_code != 0) {
        fprintf(stderr, " cause: %s\n", strerror(err->system_code));
    }
}
