#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

/**
 * @file table.h
 * @brief In-memory CSV table representation and access API.
 *
 * This module stores CSV data in a 2D dynamic structure and provides
 * access utilities for rows, columns, and cells.
 */


typedef struct Table Table;

typedef struct Cell Cell;

/**
 * @enum CellType
 * @brief Type of data stored in a cell.
 */
typedef enum {
    CELL_EMPTY = 0,   /**< Empty cell */
    CELL_NUMBER,      /**< Integer value */
    CELL_FORMULA      /**< Formula expression */
} CellType;

/**
 * @enum EvalState
 * @brief DFS evaluation state for cycle detection.
 */
typedef enum {
    NOT_VISITED = 0,  
    VISITING,        
    VISITED           
} EvalState;

/* 
 * Cell structure
 */

struct Cell {

    CellType type;  /**< Cell content type */

    union {
        int number;       /**< Integer value */
        char *formula_str; /**< Raw formula string */
    } data;

    int computed_value;  /**< Cached evaluated value */

    EvalState state;     /**< DFS state for cycle detection */

    int evaluated;       /**< Flag: 1 if computed_value is valid */
};

/**
 * Table structure
 */

struct Table {

    Cell **cells;       /**< 2D array of cells */

    int row_count;      /**< Number of rows */
    int col_count;      /**< Number of columns */

    int *row_ids;       /**< Mapping of CSV row IDs */
    char **col_names;   /**< Column names from header */
};

/* API */

Table *table_create(int rows, int cols);

void table_destroy(Table *table);

Cell *table_get_cell(Table *table, int row, int col);

int table_find_row(Table *table, int row_id);

int table_find_column(Table *table, const char *name);

void table_print_csv(const Table *table);

#endif /* TABLE_H */