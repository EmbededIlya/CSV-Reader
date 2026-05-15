/**
 * @file table.c
 * @brief Implementation of the Table API with strict encapsulation.
 * 
 * This module manages heap allocation for CSV data. Internal structures 
 * are defined here to prevent direct access from other modules.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "table.h"
#include "error.h"
#include <ctype.h>
#include "hashmap.h"
/**
 * @brief Internal Cell representation.
 */
struct Cell {
    CellType type;
    union {
        int number;
        char *formula_str;
    } data;
    int computed_value;
    EvalState state;
};

/**
 * @brief Internal Table representation.
 */
struct Table {
    Cell **cells;
    int row_count;
    int col_count;
    int *row_ids;
    char **col_names;
    HashMap *map;
};

/* --- Lifecycle Management --- */

Table *table_create(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        error_report(ERR_INVALID_ARGUMENT, "Invalid table dimensions");
        return NULL;
    }

    Table *table = (Table *)calloc(1, sizeof(Table));
    if (!table) {
        error_report(ERR_MEMORY_ALLOC, "Failed to allocate table container");
        return NULL;
    }

    table->row_count = rows;
    table->col_count = cols;

    // Allocate row pointers
    table->cells = (Cell **)calloc(rows, sizeof(Cell *));
    if (!table->cells) {
        error_report(ERR_MEMORY_ALLOC, "Failed to allocate row pointers");
        free(table);
        return NULL;
    }

    // Allocate individual rows
    for (int i = 0; i < rows; i++) {
        table->cells[i] = (Cell *)calloc(cols, sizeof(Cell));
        if (!table->cells[i]) {
            error_report(ERR_MEMORY_ALLOC, "Failed to allocate cell row");
            table_destroy(table); // Deep cleanup of already allocated memory
            return NULL;
        }
    }

    table->row_ids = (int *)calloc(rows, sizeof(int));
    table->col_names = (char **)calloc(cols, sizeof(char *));
    table->map = hashmap_create(rows * cols);
    if (!table->map) {
        error_report(ERR_MEMORY_ALLOC, "Failed to allocate hashmap");
        table_destroy(table);
        return NULL;
    }
    log_debug("Table [%dx%d] initialized successfully.", rows, cols);
    return table;
}

void table_destroy(Table *table) {
    if (!table) {
        return;
    }

    // Free cells and internal formula strings
    if (table->cells) {
        for (int i = 0; i < table->row_count; i++) {
            if (table->cells[i]) {
                for (int j = 0; j < table->col_count; j++) {
                    if (table->cells[i][j].type == CELL_FORMULA) {
                        free(table->cells[i][j].data.formula_str);
                    }
                }
                free(table->cells[i]);
            }
        }
        free(table->cells);
    }
    hashmap_destroy(table->map);
    // Free metadata
    free(table->row_ids);
    if (table->col_names) {
        for (int i = 0; i < table->col_count; i++) {
            free(table->col_names[i]);
        }
        free(table->col_names);
    }

    free(table);
    log_debug("Table resources released.");
}

/**
 * @brief Builds hashmap key in format "COL:ROW".
 */
static void table_make_key(const char *col, int row, char *out, size_t out_size)
{
    if (!col || !out || out_size == 0) {
        return;
    }

    snprintf(out, out_size, "%s:%d", col, row);
}


/* --- Getters & Data Access --- */

Cell *table_get_cell(Table *table, int row, int col) {
    if (!table) {
        return NULL;
    }

    if (row < 0 || row >= table->row_count || col < 0 || col >= table->col_count) {
        error_report(ERR_OUT_OF_BOUNDS, "Cell access out of range");
        return NULL;
    }

    return &table->cells[row][col];
}

int table_get_rows(const Table *table) {
    return table ? table->row_count : 0;
}

int table_get_cols(const Table *table) {
    return table ? table->col_count : 0;
}

/**
 * @brief Retrieves the logical type of a specific cell.
 * 
 * This function is part of the public API and handles boundary checks
 * to ensure safe access to the encapsulated data.
 * 
 * @param[in] table Constant pointer to the Table instance.
 * @param[in] row   Zero-based row index.
 * @param[in] col   Zero-based column index.
 * @return CellType The type of the cell (EMPTY, NUMBER, or FORMULA).
 *                  Returns CELL_EMPTY if indices are out of bounds or table is NULL.
 */
CellType table_get_cell_type(const Table *table, int row, int col) {
    /* 1. Validate table pointer */
    if (!table) {
        return CELL_EMPTY;
    }

    /* 2. Check row and column boundaries */
    if (row < 0 || row >= table->row_count || col < 0 || col >= table->col_count) {
        /* Optional: log an error if you want to track OOB access in tests */
        /* error_report(ERR_OUT_OF_BOUNDS, "Cell type access out of range"); */
        return CELL_EMPTY;
    }

    /* 3. Return the type from the internal cell structure */
    return table->cells[row][col].type;
}

/**
 * @brief Implementation of the cell value getter.
 * @details Validates input before accessing the internal 2D array.
 */
int table_get_cell_value(const Table *table, int row, int col) {
    /* 1. Defensive check for NULL pointer */
    if (!table) {
        return 0; 
    }

    /* 2. Boundary validation to prevent buffer overflow/illegal access */
    if (row < 0 || row >= table->row_count || col < 0 || col >= table->col_count) {
        /* Report error using the centralized error module */
        error_report(ERR_OUT_OF_BOUNDS, "Cell access out of range: [%d][%d]", row, col);
        return 0;
    }

    /* 3. Safe access to encapsulated internal data */
    /* Accessing the .computed_value field of the Cell struct at specified coordinates */
    return table->cells[row][col].computed_value;
}

/**
 * @brief Updates the content of a specific cell.
 * 
 * This implementation aligns with the header signature: 
 * (Table *table, int row, int col, const char *in_value)
 */
int table_set_cell_value(Table *table, int row, int col, const char *in_value) {

    char key[64];

    /* 1. Basic NULL pointer validation */
    if (!table || !in_value) {
        return 0; 
    }

    /* 2. Boundary validation (Bounds Checking) */
    if (row < 0 || row >= table->row_count || col < 0 || col >= table->col_count) {
        error_report(ERR_OUT_OF_BOUNDS, "Cell access out of range: [%d][%d]", row, col);
        return 0;
    }

    /* 3. Memory management: if the cell previously held a formula, free it to prevent leaks */
    if (table->cells[row][col].type == CELL_FORMULA && table->cells[row][col].data.formula_str != NULL) {
        free(table->cells[row][col].data.formula_str);
        table->cells[row][col].data.formula_str = NULL;
    }

    /* 4. Determine content type */
    if (in_value[0] == '=') {
        /* Cell is identified as a FORMULA */
        table->cells[row][col].type = CELL_FORMULA;
        
        /* Duplicate the string since in_value is const and may point to a temporary buffer */
        table->cells[row][col].data.formula_str = strdup(in_value);
        
        /* Initial computed value for formulas is 0 (until processed by the evaluator) */
        table->cells[row][col].computed_value = 0;
        
        log_debug("Cell [%d, %d] initialized as FORMULA: %s", row, col, in_value);
    } else {
        /* Cell is identified as a static NUMBER */
        table->cells[row][col].type = CELL_NUMBER;
        
        /* Convert the input string to an integer */
        table->cells[row][col].computed_value = atoi(in_value);
        
        log_debug("Cell [%d, %d] set as NUMBER: %d", row, col, table->cells[row][col].computed_value);
    }

    /* 5. Update hashmap for quick lookup */
    table_make_key(table->col_names[col], row, key, sizeof(key));
    hashmap_put(table->map, key, row, col);

    return 1;
}

/**
 * @brief Retrieves the raw formula string from a cell if it contains a formula.
 *
 * This function provides safe read-only access to the internal formula representation
 * without exposing the internal Cell structure outside the Table module.
 *
 * @param[in] table Pointer to the constant Table instance.
 * @param[in] row   Zero-based row index.
 * @param[in] col   Zero-based column index.
 * @return const char* Pointer to internal formula string, or NULL if:
 *         - table is NULL
 *         - indices are out of bounds
 *         - cell does not contain a formula
 */
const char* table_get_cell_formula(const Table *table, int row, int col)
{
    /* 1. Basic NULL pointer validation */
    if (!table) {
        return NULL;
    }

    /* 2. Boundary validation (Bounds Checking) */
    if (row < 0 || row >= table->row_count ||
        col < 0 || col >= table->col_count)
    {
        return NULL;
    }

    /* 3. Type check: ensure the cell actually contains a formula */
    if (table->cells[row][col].type != CELL_FORMULA) {
        return NULL;
    }

    /* 4. Safe access to internal formula storage */
    return table->cells[row][col].data.formula_str;
}

/**
 * @brief Updates the computed numeric value of a specific cell.
 *
 * This function is used by the evaluator module to store the result
 * of formula computation inside the table without exposing internal structure.
 *
 * @param[in,out] table Pointer to the Table instance.
 * @param[in] row       Zero-based row index.
 * @param[in] col       Zero-based column index.
 * @param[in] value     Computed integer result of formula evaluation.
 * @return int          1 on success, 0 on failure (NULL table or OOB access).
 */
int table_set_cell_computed_value(Table *table, int row, int col, int value)
{
    /* 1. Basic NULL pointer validation */
    if (!table) {
        return 0;
    }

    /* 2. Boundary validation (Bounds Checking) */
    if (row < 0 || row >= table->row_count ||
        col < 0 || col >= table->col_count)
    {
        return 0;
    }

    /* 3. Write computed value into internal cell storage */
    table->cells[row][col].computed_value = value;

    /* 4. Successful update */
    return 1;
}

/**
 * @brief Internal helper: resolves logical (col + row) into table indices.
 *
 * Uses hashmap lookup to translate human-readable coordinates into
 * internal matrix positions.
 *
 * @param[in]  table    Pointer to Table instance.
 * @param[in]  col      Column name (e.g. "A", "B", "Cell").
 * @param[in]  row      Row index.
 * @param[out] out_row  Resolved row index.
 * @param[out] out_col  Resolved column index.
 * @return int          1 on success, 0 on failure.
 */
static int table_find_by_name(const Table *table,
                    const char *col,
                               int row,
                               int *out_row,
                               int *out_col)
{
    if (!table || !table->map || !col || !out_row || !out_col) {
        return 0;
    }

    char key[64];
    table_make_key(col, row, key, sizeof(key));

    return hashmap_get(table->map, key, out_row, out_col);
}

/**
 * @brief Retrieves computed value of a cell by logical name.
 *
 * Converts (col + row) into internal indices and returns stored value.
 *
 * @param[in] table Pointer to Table instance.
 * @param[in] col   Column name.
 * @param[in] row   Row index.
 * @return int      Computed value, or 0 on failure.
 */
int table_get_cell_value_by_name(const Table *table,
                                const char *col,
                                int row)
{
    int r = 0;
    int c = 0;

    /* 1. Resolve logical address */
    if (!table_find_by_name(table, col, row, &r, &c)) {
        return 0;
    }

    /* 2. Return computed value */
    return table->cells[r][c].computed_value;
}

/**
 * @brief Retrieves cell type by logical name.
 *
 * @param[in] table Pointer to Table instance.
 * @param[in] col   Column name.
 * @param[in] row   Row index.
 * @return CellType Cell type or CELL_EMPTY on failure.
 */
CellType table_get_cell_type_by_name(const Table *table,
                                    const char *col,
                                    int row)
{
    int r = 0;
    int c = 0;

    /* 1. Resolve logical address */
    if (!table_find_by_name(table, col, row, &r, &c)) {
        return CELL_EMPTY;
    }

    /* 2. Return cell type */
    return table->cells[r][c].type;
}

/**
 * @brief Retrieves formula string by logical name.
 *
 * Returns internal formula pointer if cell contains a formula.
 *
 * @param[in] table Pointer to Table instance.
 * @param[in] col   Column name.
 * @param[in] row   Row index.
 * @return const char* Formula string or NULL.
 */
const char* table_get_cell_formula_by_name(const Table *table,
                                          const char *col,
                                          int row)
{
    int r = 0;
    int c = 0;

    /* 1. Resolve logical address */
    if (!table_find_by_name(table, col, row, &r, &c)) {
        return NULL;
    }

    /* 2. Ensure cell contains formula */
    if (table->cells[r][c].type != CELL_FORMULA) {
        return NULL;
    }

    /* 3. Return formula string */
    return table->cells[r][c].data.formula_str;
}

/**
 * @brief Updates computed value of a cell by logical name.
 *
 * Used by evaluator after formula execution.
 *
 * @param[in,out] table Pointer to Table instance.
 * @param[in]     col   Column name.
 * @param[in]     row   Row index.
 * @param[in]     value Computed value.
 * @return int          1 on success, 0 on failure.
 */
int table_set_cell_computed_value_by_name(Table *table,
                                         const char *col,
                                         int row,
                                         int value)
{
    int r = 0;
    int c = 0;

    /* 1. Resolve logical address */
    if (!table_find_by_name(table, col, row, &r, &c)) {
        return 0;
    }

    /* 2. Store computed value */
    table->cells[r][c].computed_value = value;

    return 1;
}

/* --- CSV Output --- */

void table_print_csv(const Table *table) {
    if (!table || !table->cells) {
        return;
    }

    // Header row
    if (table->col_names) {
        printf(",");
        for (int c = 0; c < table->col_count; c++) {
            printf("%s%s", table->col_names[c] ? table->col_names[c] : "", 
                   (c < table->col_count - 1) ? "," : "");
        }
        printf("\n");
    }

    // Data rows
    for (int r = 0; r < table->row_count; r++) {
        // Row ID
        printf("%d,", table->row_ids ? table->row_ids[r] : r);

        for (int c = 0; c < table->col_count; c++) {
            const Cell *cell = &table->cells[r][c];

            if (cell->type == CELL_NUMBER) {
                printf("%d", cell->computed_value);
            } else if (cell->type == CELL_FORMULA) {
                // If it's still a formula, it means evaluator hasn't processed it
                printf("#N/A"); 
            } else {
                printf("0");
            }

            if (c < table->col_count - 1) {
                printf(",");
            }
        }
        printf("\n");
    }
}




