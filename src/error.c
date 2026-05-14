/**
 * @file error.c
 * @brief Implementation of error reporting and debugging utilities.
 */

#include "error.h"
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief Human-readable descriptions for each ErrorCode.
 */
static const char* const error_strings[] = {
    "Success",
    "Memory allocation failed",
    "Index out of bounds",
    "Invalid CSV format",
    "Formula syntax error",
    "Circular dependency detected",
    "Division by zero",
    "Invalid argument",
    "Invalid dimension for cell"
};

/**
 * @brief Converts an ErrorCode to its string representation.
 * 
 * @param code The error code to convert.
 * @return const char* Description of the error or "Unknown error".
 */
static const char* error_to_string(ErrorCode code) {
    const int num_errors = (int)(sizeof(error_strings) / sizeof(error_strings[0]));
    
    if ((int)code < 0 || (int)code >= num_errors) {
        return "Unknown error";
    }
    return error_strings[code];
}

void error_report_impl(ErrorCode code, const char *file, int line, 
                       const char *func, const char *format, ...) {
    if (code == ERR_OK) return;

    /* Print the primary error header */
    fprintf(stderr, "[ERROR] %s\n", error_to_string(code));
    fprintf(stderr, "        Location: %s:%d\n", file ? file : "unknown", line);
    fprintf(stderr, "        Function: %s\n", func ? func : "unknown");
    
    /* Print the variadic context message */
    fprintf(stderr, "        Context:  ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void log_debug(const char *format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    printf("[DEBUG] ");
    vfprintf(stdout, format, args);
    printf("\n");
    va_end(args);
#else
    /* Prevent "unused parameter" warning when not in DEBUG mode */
    (void)format; 
#endif
}