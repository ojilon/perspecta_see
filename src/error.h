#ifndef PERSPECTA_ERROR_H
#define PERSPECTA_ERROR_H

#include <stdbool.h>

typedef enum {
    ERR_NONE = 0, /*0 initialized error means no error */

    ERR_INVALID_ARGUMENT,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    ERR_INTERNAL
} ErrorCode;

typedef struct {
    ErrorCode code;
    const char *message; /*musr be s string literal */
    int system_code;
    const char *file; /*filed by macro */
    int line; /*filled by macro*/
} Error;

Error error_make(ErrorCode code, const char *message, const char *file, int line);
bool error_is_ok(const Error *err);
const char *error_name(ErrorCode code);
void error_print(const Error *err);

#define ERROR(code, message) error_make((code), (message), __FILE__, __LINE__)
#define ERROR_OK ((Error){0})

#endif

