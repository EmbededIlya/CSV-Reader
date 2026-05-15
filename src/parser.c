/**
 * @file parser.c
 * @brief Implementation of spreadsheet formula parser with strict encapsulation.
 *
 * This module converts raw formula strings into internal structured objects.
 * Internal representations are hidden from external modules.
 */

#include "parser.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/**
 * @brief Maximum token length for operands.
 */
#define PARSER_TOKEN_MAX_LEN 128

/**
 * @brief Internal representation of a formula argument.
 */
struct Argument {
    ArgumentType type;

    union {
        int number;

        struct {
            char col[8];
            int row;
        } cell;

    } data;
};

/**
 * @brief Internal representation of a binary formula.
 */
struct Formula {
    Argument *left;
    Argument *right;
    char op;
};

/* --- Internal helpers --- */

/**
 * @brief Checks if character is arithmetic operator.
 */
static int parser_is_operator(char c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

/**
 * @brief Safe integer parsing with validation.
 */
static int parser_parse_int(const char *str, int *value)
{
    if (!str || !value) {
        log_debug("parser_parse_int: NULL argument");
        return -1;
    }

    errno = 0;  // <<< ВАЖНО

    char *end = NULL;
    long val = strtol(str, &end, 10);

    /* 1. invalid format */
    if (end == str || *end != '\0') {
        log_debug("Invalid integer token: %s", str);
        return -1;
    }

    /* 2. overflow/underflow from strtol */
    if (errno == ERANGE) {
        log_debug("strtol overflow: %s", str);
        return -1;
    }

    /* 3. explicit int range check */
    if (val > INT_MAX || val < INT_MIN) {
        log_debug("Integer overflow: %s", str);
        return -1;
    }

    *value = (int)val;
    return 0;
}

/**
 * @brief Creates Argument from token.
 */
static Argument* parser_create_argument(const char *token)
{
    if (!token || *token == '\0') {
        log_debug("Empty token in argument");
        return NULL;
    }

    Argument *arg = (Argument *)calloc(1, sizeof(Argument));
    if (!arg) {
        log_debug("calloc failed for Argument");
        return NULL;
    }

    /* --- NUMBER --- */
    if (isdigit((unsigned char)token[0]) ||
        (token[0] == '-' && isdigit((unsigned char)token[1])))
    {
        arg->type = ARG_NUMBER;

        if (parser_parse_int(token, &arg->data.number) != 0) {
            free(arg);
            return NULL;
        }

        return arg;
    }

    /* --- CELL --- */
    arg->type = ARG_CELL;

    size_t i = 0;

    while (isalpha((unsigned char)token[i]) &&
           i < sizeof(arg->data.cell.col) - 1)
    {
        arg->data.cell.col[i] =
            (char)toupper((unsigned char)token[i]);
        i++;
    }

    arg->data.cell.col[i] = '\0';

    if (i == 0) {
        log_debug("Missing column in cell: %s", token);
        free(arg);
        return NULL;
    }

    if (parser_parse_int(&token[i], &arg->data.cell.row) != 0 ||
        arg->data.cell.row <= 0)
    {
        log_debug("Invalid row in cell: %s", token);
        free(arg);
        return NULL;
    }

    return arg;
}

/* --- Public API --- */

int parser_is_formula(const char *str)
{
    if (!str) {
        return 0;
    }

    return (str[0] == '=');
}

/* --- Core parser --- */

Formula* parser_parse_formula(const char *str)
{
    if (!str) {
        log_debug("NULL formula input");
        return NULL;
    }

    if (!parser_is_formula(str)) {
        log_debug("Missing '=' prefix");
        return NULL;
    }

    const char *p = str + 1;

    char left[PARSER_TOKEN_MAX_LEN] = {0};
    char right[PARSER_TOKEN_MAX_LEN] = {0};

    size_t i = 0;

    /* --- LEFT operand --- */
    while (*p && !parser_is_operator(*p)) {

        if (i >= PARSER_TOKEN_MAX_LEN - 1) {
            log_debug("Left operand overflow");
            return NULL;
        }

        left[i++] = *p++;
    }

    left[i] = '\0';

    if (*p == '\0') {
        log_debug("Missing operator");
        return NULL;
    }

    char op = *p++;

    if (!parser_is_operator(op)) {
        log_debug("Invalid operator");
        return NULL;
    }

    /* --- RIGHT operand --- */
    i = 0;

    while (*p) {

        if (i >= PARSER_TOKEN_MAX_LEN - 1) {
            log_debug("Right operand overflow");
            return NULL;
        }

        right[i++] = *p++;
    }

    right[i] = '\0';

    if (left[0] == '\0' || right[0] == '\0') {
        log_debug("Empty operand detected");
        return NULL;
    }

    Formula *f = (Formula *)calloc(1, sizeof(Formula));
    if (!f) {
        log_debug("calloc failed for Formula");
        return NULL;
    }

    f->op = op;

    f->left = parser_create_argument(left);
    f->right = parser_create_argument(right);

    if (!f->left || !f->right) {
        log_debug("Operand parsing failed");
        free(f->left);
        free(f->right);
        free(f);
        return NULL;
    }

    return f;
}

/* --- Destructor --- */

void parser_destroy_formula(Formula *f)
{
    if (!f) {
        return;
    }

    free(f->left);
    free(f->right);
    free(f);

    log_debug("Formula destroyed");
}

/* --- Accessors --- */

char parser_get_operator(const Formula *f)
{
    return f ? f->op : '\0';
}

Argument* parser_get_left_operand(const Formula *f)
{
    return f ? f->left : NULL;
}

Argument* parser_get_right_operand(const Formula *f)
{
    return f ? f->right : NULL;
}

ArgumentType parser_get_arg_type(const Argument *arg)
{
    return arg ? arg->type : ARG_NUMBER;
}

int parser_get_arg_number(const Argument *arg)
{
    return arg ? arg->data.number : 0;
}

const char* parser_get_arg_cell_col(const Argument *arg)
{
    return arg ? arg->data.cell.col : "";
}

int parser_get_arg_cell_row(const Argument *arg)
{
    return arg ? arg->data.cell.row : 0;
}