/**
 * @file error.h
 * @brief Error handling and logging system for the CSV Reader project.
 * 
 * Defines error codes and provides macros and functions for reporting
 * runtime errors and debug information to stderr/stdout.
 */

#ifndef ERROR_H
#define ERROR_H

/**
 * @enum ErrorCode
 * @brief Standardized error codes for the application.
 */
typedef enum {
    ERR_OK = 0,               /**< No error / Success */
    ERR_MEMORY_ALLOC,        /**< Failed to allocate memory on heap */
    ERR_OUT_OF_BOUNDS,       /**< Array or table index out of range */
    ERR_INVALID_CSV,         /**< Malformed CSV input detected */
    ERR_FORMULA_SYNTAX,      /**< Error in formula expression */
    ERR_CIRCULAR_DEP,        /**< Circular reference between cells */
    ERR_DIV_BY_ZERO,         /**< Mathematical division by zero */
    ERR_INVALID_ARGUMENT,    /**< Null pointer or invalid parameter passed to function */
    ERR_INVALID_ZERO_CELL    /**< Dimension error specifically for cell operations */
} ErrorCode;

/**
 * @brief Variadic macro for error reporting.
 * 
 * Automatically captures the file name, line number, and function name
 * where the error occurred, then passes them to the implementation function.
 * 
 * @param code The @ref ErrorCode representing the error type.
 * @param ...  Formatted string and arguments (printf-style) providing context.
 */
#define error_report(code, ...) \
    error_report_impl(code, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Implementation of the error reporting logic.
 * 
 * Should not be called directly. Use the @ref error_report macro instead.
 * 
 * @param code   Error code.
 * @param file   Name of the source file.
 * @param line   Line number in the source file.
 * @param func   Name of the function where the error occurred.
 * @param format Printf-style format string.
 * @param ...    Arguments for the format string.
 */
void error_report_impl(ErrorCode code, const char *file, int line, 
                       const char *func, const char *format, ...);

/**
 * @brief Logs a debug message to standard output.
 * 
 * Only active if the DEBUG macro is defined during compilation.
 * 
 * @param format Printf-style format string.
 * @param ...    Arguments for the format string.
 */
void log_debug(const char *format, ...);

#endif /* ERROR_H */