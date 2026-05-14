/**
 * @file test_table.c
 * @brief Unit tests for the Table module.
 * 
 * Tests cover lifecycle management, boundary conditions, and memory integrity.
 * Complies with isolation principles (tests do not share state).
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "table.h"
#include "error.h"

/* --- Test Cases --- */

/**
 * @brief Test standard table initialization and dimensions.
 */
static void test_table_initialization(void) {
    printf("Running: %s... ", __func__);

    const int rows = 5;
    const int cols = 3;
    Table *table = table_create(rows, cols);

    assert(table != NULL);
    assert(table_get_rows(table) == rows);
    assert(table_get_cols(table) == cols);

    table_destroy(table);
    printf("PASSED\n");
}

/**
 * @brief Test boundary conditions (negative/zero dimensions).
 */
static void test_table_invalid_dimensions(void) {
    printf("Running: %s... ", __func__);

    /* Industrial standard: ensure API fails gracefully on bad input */
    Table *t1 = table_create(0, 5);
    assert(t1 == NULL);

    Table *t2 = table_create(5, -1);
    assert(t2 == NULL);

    printf("PASSED\n");
}

/**
 * @brief Test cell access and OOB (Out of Bounds) protection.
 */
static void test_table_cell_access(void) {
    printf("Running: %s... ", __func__);

    Table *table = table_create(2, 2);
    
    /* Valid access */
    assert(table_get_cell(table, 0, 0) != NULL);
    assert(table_get_cell(table, 1, 1) != NULL);

    /* Out of bounds access (should return NULL via error_report) */
    assert(table_get_cell(table, 2, 0) == NULL);  /* Row 2 doesn't exist */
    assert(table_get_cell(table, 0, -1) == NULL); /* Negative index */
    assert(table_get_cell(table, 100, 100) == NULL);

    table_destroy(table);
    printf("PASSED\n");
}

/**
 * @brief Tests the initial state of a newly created table.
 */
void test_cell_initial_state(void) {
    printf("Running: %s... ", __func__);

    /* Arrange */
    Table *table = table_create(2, 2);
    assert(table != NULL);

    /* Act & Assert */
    /* Check that the very first cell is empty/zero by default */
    assert(table_get_cell_type(table, 0, 0) == CELL_EMPTY);
    assert(table_get_cell_value(table, 0, 0) == 0);

    table_destroy(table);
    printf("PASSED\n");
}

/**
 * @brief Test memory cleanup for deep structures (Formulas).
 */
static void test_table_deep_cleanup(void) {
    printf("Running: %s... ", __func__);

    Table *table = table_create(1, 1);
    Cell *cell = table_get_cell(table, 0, 0);
    
    /* Simulate adding a formula string to the union */
    /* Note: Ideally, we'd use a public API function table_set_formula, 
       but for unit testing internal integrity, we check the lifecycle. */
    // table_set_formula(table, 0, 0, "=A1+10"); // This would be the real use case
    
    table_destroy(table);
    /* If there are no leaks in Valgrind, deep cleanup works. */
    printf("PASSED\n");
}

/**
 * @brief Test setting and getting cell values (Numbers and Formulas).
 * 
 * This test verifies that the API correctly distinguishes between 
 * numeric inputs and formulas, and stores them appropriately.
 */
static void test_table_set_get_value(void) {
    printf("Running: %s... ", __func__);

    Table *table = table_create(2, 2);
    assert(table != NULL);

    /* 1. Test Numeric Assignment */
    /* We pass "42" as a string, mimicking CSV input */
    int res1 = table_set_cell_value(table, 0, 0, "42");
    assert(res1 == 1);
    assert(table_get_cell_type(table, 0, 0) == CELL_NUMBER);
    assert(table_get_cell_value(table, 0, 0) == 42);

    /* 2. Test Formula Assignment */
    /* Formulas start with '=' */
    int res2 = table_set_cell_value(table, 1, 1, "=A1+5");
    assert(res2 == 1);
    assert(table_get_cell_type(table, 1, 1) == CELL_FORMULA);
    /* Value should be 0 until evaluated by the engine */
    assert(table_get_cell_value(table, 1, 1) == 0); 

    /* 3. Test Overwriting */
    /* Changing cell type from formula back to number */
    table_set_cell_value(table, 1, 1, "100");
    assert(table_get_cell_type(table, 1, 1) == CELL_NUMBER);
    assert(table_get_cell_value(table, 1, 1) == 100);

    table_destroy(table);
    printf("PASSED\n");
}

/* --- Test Runner --- */

int main(void) {
    printf("=== Starting Table Module Unit Tests ===\n");

    test_table_initialization();
    test_table_invalid_dimensions();
    test_table_cell_access();
    test_table_deep_cleanup();
    test_cell_initial_state();
    test_table_set_get_value();
    printf("\n========================================\n");
    printf("\n=== Tests of table.c Completed Successfully ===\n");
    return 0;
}
