/**
 * @file parser.h
 * @brief Encapsulated Formula Parser API for CSV expression processing.
 *
 * Implements the Pimpl (Pointer to Implementation) / Opaque Pointer pattern
 * to hide internal AST structures and ensure strict encapsulation of parsing logic.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "error.h"

/** @brief Forward declaration of the internal Formula structure. */
typedef struct Formula Formula;

/** @brief Forward declaration of the internal Argument structure. */
typedef struct Argument Argument;

/**
 * @enum ArgumentType
 * @brief Logical type of a formula operand.
 */
typedef enum {
    ARG_NUMBER = 0,   /**< Constant integer value */
    ARG_CELL          /**< Cell reference (e.g. A1, B12) */
} ArgumentType;

/* --- Lifecycle Operations --- */

/**
 * @brief Parses a raw formula string into internal Formula representation.
 *
 * Supported format:
 * "=A1+10", "=5+B2", "=A1-B3"
 *
 * @param str Input string (must start with '=')
 * @return Formula* Pointer to allocated Formula object, or NULL on failure
 */
Formula* parser_parse_formula(const char *str);

/**
 * @brief Releases all memory associated with Formula object.
 *
 * Performs deep cleanup of internal operands.
 *
 * @param f Pointer to Formula instance (safe to pass NULL)
 */
void parser_destroy_formula(Formula *f);

/* --- Validation API --- */

/**
 * @brief Checks whether a string is a formula.
 *
 * @param str Input string
 * @return int 1 if formula (starts with '='), 0 otherwise
 */
int parser_is_formula(const char *str);

/* --- Accessor API --- */

/**
 * @brief Returns arithmetic operator of a formula.
 *
 * @param f Constant pointer to Formula instance
 * @return char '+', '-', '*', '/', or '\0' on invalid input
 */
char parser_get_operator(const Formula *f);

/**
 * @brief Returns left operand of a formula.
 *
 * @param f Pointer to Formula instance
 * @return Argument* Pointer to left operand or NULL
 */
Argument* parser_get_left_operand(const Formula *f);

/**
 * @brief Returns right operand of a formula.
 *
 * @param f Pointer to Formula instance
 * @return Argument* Pointer to right operand or NULL
 */
Argument* parser_get_right_operand(const Formula *f);

/**
 * @brief Returns type of a formula argument.
 *
 * @param arg Pointer to Argument instance
 * @return ArgumentType Type of operand (NUMBER or CELL)
 */
ArgumentType parser_get_arg_type(const Argument *arg);

/**
 * @brief Returns numeric value of argument (valid only if ARG_NUMBER).
 *
 * @param arg Pointer to Argument instance
 * @return int Parsed integer value
 */
int parser_get_arg_number(const Argument *arg);

/**
 * @brief Returns column part of a cell reference.
 *
 * @param arg Pointer to Argument instance
 * @return const char* Column string (e.g. "A", "BC")
 */
const char* parser_get_arg_cell_col(const Argument *arg);

/**
 * @brief Returns row index of a cell reference.
 *
 * @param arg Pointer to Argument instance
 * @return int 1-based row index
 */
int parser_get_arg_cell_row(const Argument *arg);

#endif /* PARSER_H */