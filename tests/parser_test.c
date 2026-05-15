/**
 * @file test_parser.c
 * @brief Unit tests for the Parser module.
 *
 * Tests cover formula parsing, operand detection, error handling,
 * and memory integrity of the parser subsystem.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "error.h"

/* --- Helpers --- */

static void assert_formula_basic(Formula *f)
{
    assert(f != NULL);
    assert(parser_get_operator(f) != '\0');
    assert(parser_get_left_operand(f) != NULL);
    assert(parser_get_right_operand(f) != NULL);
}

/* --- VALID CASES --- */

/**
 * Numbers are valid operands in spreadsheet formulas
 */
static void test_parser_numeric_formula(void)
{
    printf("Running: %s... ", __func__);

    Formula *f = parser_parse_formula("=10+20");

    assert_formula_basic(f);
    assert(parser_get_operator(f) == '+');

    /* numbers are valid operands */
    assert(parser_get_arg_number(parser_get_left_operand(f)) == 10);
    assert(parser_get_arg_number(parser_get_right_operand(f)) == 20);

    parser_destroy_formula(f);

    printf("PASSED\n");
}

/**
 * Cell + Cell expression
 */
static void test_parser_cell_formula(void)
{
    printf("Running: %s... ", __func__);

    Formula *f = parser_parse_formula("=A1+B2");

    assert_formula_basic(f);
    assert(parser_get_operator(f) == '+');

    assert(strcmp(parser_get_arg_cell_col(parser_get_left_operand(f)), "A") == 0);
    assert(parser_get_arg_cell_row(parser_get_left_operand(f)) == 1);

    assert(strcmp(parser_get_arg_cell_col(parser_get_right_operand(f)), "B") == 0);
    assert(parser_get_arg_cell_row(parser_get_right_operand(f)) == 2);

    parser_destroy_formula(f);

    printf("PASSED\n");
}

/**
 * Mixed number + cell
 */
static void test_parser_mixed_formula(void)
{
    printf("Running: %s... ", __func__);

    Formula *f = parser_parse_formula("=A10+5");

    assert_formula_basic(f);
    assert(parser_get_operator(f) == '+');

    /* left = cell */
    assert(strcmp(parser_get_arg_cell_col(parser_get_left_operand(f)), "A") == 0);
    assert(parser_get_arg_cell_row(parser_get_left_operand(f)) == 10);

    /* right = number */
    assert(parser_get_arg_number(parser_get_right_operand(f)) == 5);

    parser_destroy_formula(f);

    printf("PASSED\n");
}

/* --- INVALID FORMULAS --- */

static void test_parser_invalid_formulas(void)
{
    printf("Running: %s... ", __func__);

    assert(parser_parse_formula("A1+5") == NULL);     /* missing '=' */
    assert(parser_parse_formula("=A1B2") == NULL);    /* invalid syntax */
    assert(parser_parse_formula("") == NULL);
    assert(parser_parse_formula(NULL) == NULL);

    printf("PASSED\n");
}

/**
 * Invalid cell formats only
 */
static void test_parser_invalid_cell(void)
{
    printf("Running: %s... ", __func__);

    assert(parser_parse_formula("=A0+5") == NULL);    /* row must be > 0 */
    assert(parser_parse_formula("=A#1+5") == NULL);   /* invalid symbol */
    assert(parser_parse_formula("=AA+5") == NULL);    /* missing row */

    printf("PASSED\n");
}

/**
 * Overflow / long input protection
 */
static void test_parser_overflow(void)
{
    printf("Running: %s... ", __func__);

    char long_input[300];
    long_input[0] = '=';

    for (int i = 1; i < 299; i++) {
        long_input[i] = 'A';
    }
    long_input[299] = '\0';

    assert(parser_parse_formula(long_input) == NULL);

    printf("PASSED\n");
}

/* --- OPERATORS --- */

static void test_parser_operators(void)
{
    printf("Running: %s... ", __func__);

    Formula *f1 = parser_parse_formula("=1+2");
    Formula *f2 = parser_parse_formula("=1-2");
    Formula *f3 = parser_parse_formula("=1*2");
    Formula *f4 = parser_parse_formula("=1/2");

    assert(parser_get_operator(f1) == '+');
    assert(parser_get_operator(f2) == '-');
    assert(parser_get_operator(f3) == '*');
    assert(parser_get_operator(f4) == '/');

    parser_destroy_formula(f1);
    parser_destroy_formula(f2);
    parser_destroy_formula(f3);
    parser_destroy_formula(f4);

    printf("PASSED\n");
}

/* --- Token boundaries and whitespace handling --- */
static void test_parser_token_boundaries(void)
{
    printf("Running: %s... ", __func__);

    /* spaces are NOT supported */
    assert(parser_parse_formula("= A1+5") == NULL);
    assert(parser_parse_formula("=A1 +5") == NULL);
    assert(parser_parse_formula("=A1+ 5") == NULL);
    assert(parser_parse_formula("= A1 + 5") == NULL);

    /* unexpected separators */
    assert(parser_parse_formula("=A1\t+5") == NULL);
    assert(parser_parse_formula("=A1\n+5") == NULL);

    printf("PASSED\n");
}

/* --- Invalid numeric tokens --- */
static void test_parser_invalid_numbers(void)
{
    printf("Running: %s... ", __func__);

    /* overflow */
    assert(parser_parse_formula("=999999999999999999999+1") == NULL);

    /* invalid numeric tokens */
    assert(parser_parse_formula("=12a+5") == NULL);
    assert(parser_parse_formula("=--5+2") == NULL);
    assert(parser_parse_formula("=+5+2") == NULL);

    printf("PASSED\n");
}

/* --- Mixed syntax errors --- */
static void test_parser_mixed_errors(void)
{
    printf("Running: %s... ", __func__);

    /* broken cell + number mix */
    assert(parser_parse_formula("=A#1+5") == NULL);
    assert(parser_parse_formula("=A1+#5") == NULL);

    /* multiple syntax violations */
    assert(parser_parse_formula("=A1++B2") == NULL);
    assert(parser_parse_formula("=A1+-B2") == NULL);

    /* missing operand */
    assert(parser_parse_formula("=A1+") == NULL);
    assert(parser_parse_formula("=+A1") == NULL);

    printf("PASSED\n");
}

/* --- Edge grammar cases --- */
static void test_parser_edge_grammar(void)
{
    printf("Running: %s... ", __func__);

    /* empty-like constructs */
    assert(parser_parse_formula("=") == NULL);
    assert(parser_parse_formula("=+") == NULL);

    /* malformed structure */
    assert(parser_parse_formula("=A1B2+3") == NULL);
    assert(parser_parse_formula("=A1B2C3") == NULL);

    /* repeated operators */
    assert(parser_parse_formula("=A1++A2") == NULL);
    assert(parser_parse_formula("=A1//A2") == NULL);

    /* weird but common user mistakes */
    assert(parser_parse_formula("=A1 + B2") == NULL);
    assert(parser_parse_formula("=A1+ B2") == NULL);

    printf("PASSED\n");
}



/* --- RUNNER --- */

int main(void)
{
    printf("=== Starting Parser Module Unit Tests ===\n\n");

    test_parser_numeric_formula();
    test_parser_cell_formula();
    test_parser_mixed_formula();
    test_parser_invalid_formulas();
    test_parser_invalid_cell();
    test_parser_overflow();
    test_parser_operators();
    test_parser_token_boundaries();
    test_parser_invalid_numbers();
    test_parser_mixed_errors();
    test_parser_edge_grammar();
    printf("\n========================================\n");
    printf("=== Tests of parser.c Completed Successfully ===\n");
    return 0;
}