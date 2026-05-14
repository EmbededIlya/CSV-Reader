/**
 * @file table.h
 * @brief Encapsulated Table API for CSV data processing.
 * 
 * Implements the Pimpl (Pointer to implementation) / Opaque Pointer pattern
 * to hide internal data structures, ensuring binary stability and 
 * preventing unauthorized modification of table metadata.
 */

#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include "error.h"

/** @brief Forward declaration of the internal Table structure. */
typedef struct Table Table;

/** @brief Forward declaration of the internal Cell structure. */
typedef struct Cell Cell;

/**
 * @enum CellType
 * @brief Logical types of data a cell can hold.
 */
typedef enum {
    CELL_EMPTY = 0,   /**< Default state: no data provided */
    CELL_NUMBER,      /**< Static integer value */
    CELL_FORMULA      /**< Unprocessed expression starting with '=' */
} CellType;

typedef enum {
    STATE_NOT_VISITED = 0, /**< Cell has not been evaluated yet */
    STATE_VISITING,        /**< Cell is currently being evaluated (DFS) */
    STATE_VISITED         /**< Cell has been fully evaluated */
} EvalState;

/* --- Lifecycle Operations --- */

/**
 * @brief Instantiates a new Table in heap memory.
 * 
 * @param rows Positive number of rows.
 * @param cols Positive number of columns.
 * @return Table* Pointer to the instance, or NULL on allocation failure.
 */
Table *table_create(int rows, int cols);

/**
 * @brief Releases all memory resources associated with the Table.
 * 
 * Handles deep cleanup of nested pointers (cell data, column names, etc.).
 * @param table The table instance to destroy. Safe to pass NULL.
 */
void table_destroy(Table *table);


/* --- Metadata Accessors --- */

/**
 * @brief Returns the total number of rows in the table.
 * 
 * @param table Constant pointer to the table instance.
 * @return int Number of rows, or 0 if table is NULL.
 */
int table_get_rows(const Table *table);

/**
 * @brief Returns the total number of columns in the table.
 * 
 * @param table Constant pointer to the table instance.
 * @return int Number of columns, or 0 if table is NULL.
 */
int table_get_cols(const Table *table);

/* --- Data Interaction API --- */

/**
 * @brief Provides access to a cell via its internal 2D coordinates.
 * 
 * @param table Pointer to the table instance.
 * @param row_idx Zero-based row index.
 * @param col_idx Zero-based column index.
 * @return Cell* Pointer to the cell object, or NULL if indices are invalid.
 */
Cell* table_get_cell(Table *table, int row_idx, int col_idx);

/**
 * @brief Retrieves the logical type of a specific cell.
 * 
 * Used to distinguish between raw numbers, formulas, and empty cells.
 * 
 * @param table Constant pointer to the table instance.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @return CellType The type of the cell. Returns CELL_EMPTY if indices are invalid.
 */
CellType table_get_cell_type(const Table *table, int row, int col);

/**
 * @brief Retrieves the computed integer value of a specific cell.
 * 
 * This function provides a safe way to access cell data from an encapsulated Table.
 * It performs bounds checking and handles null pointers gracefully.
 *
 * @param[in] table  Pointer to the constant Table instance.
 * @param[in] row    The zero-based row index.
 * @param[in] col    The zero-based column index.
 * @return int       The computed value of the cell. Returns 0 if indices 
 *                   are out of bounds or if the table pointer is NULL.
 */
int table_get_cell_value(const Table *table, int row, int col);

/**
 * @brief Updates the content of a specific cell.
 * 
 * Automatically detects if the input string is a formula (starts with '=') 
 * or a raw number. Handles memory allocation for formula strings internally.
 * 
 * @param[in,out] table Pointer to the table instance.
 * @param[in] row       Zero-based row index.
 * @param[in] col       Zero-based column index.
 * @param[in] in_value  String representation of the value (e.g., "42" or "=A1+B2").
 * @return int          1 on success, 0 on failure (OOB or memory error).
 */

int table_set_cell_value(Table *table, int row, int col, const char *in_value);
/**
 * @brief Renders the entire table to standard output in CSV format.
 * 
 * Ensures thread-safe, read-only access to the table during the process.
 * @param table Constant pointer to the table instance.
 */
void table_print_csv(const Table *table);

#endif /* TABLE_H */