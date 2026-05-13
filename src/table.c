
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "table.h"
#include "error.h"

/*
    =========================
    Table creation
    =========================
*/

Table *table_create(int rows, int cols)
{
    if (rows <= 0 || cols <= 0) {
        return NULL;
    }

    Table *table = (Table *)malloc(sizeof(Table));
    if (!table) {
        return NULL;
    }

    table->row_count = rows;
    table->col_count = cols;

    table->cells = (Cell **)malloc(rows * sizeof(Cell *));
    if (!table->cells) {
        free(table);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        table->cells[i] = (Cell *)calloc(cols, sizeof(Cell));
        if (!table->cells[i]) {

            // cleanup on failure
            for (int j = 0; j < i; j++) {
                free(table->cells[j]);
            }
            free(table->cells);
            free(table);
            return NULL;
        }
    }

    table->row_ids = NULL;
    table->col_names = NULL;

    return table;
}

/*
    =========================
    Memory cleanup
    =========================
*/

void table_destroy(Table *table)
{
    if (!table) {
        return;
    }

    if (table->cells) {
        for (int i = 0; i < table->row_count; i++) {
            free(table->cells[i]);
        }
        free(table->cells);
    }

    if (table->row_ids) {
        free(table->row_ids);
    }

    if (table->col_names) {
        free(table->col_names);
    }

    free(table);
}

/*
    =========================
    Cell access
    =========================
*/

Cell *table_get_cell(Table *table, int row, int col)
{
    if (!table) {
        return NULL;
    }

    if (row < 0 || row >= table->row_count) {
        return NULL;
    }

    if (col < 0 || col >= table->col_count) {
        return NULL;
    }

    return &table->cells[row][col];
}

/*
    =========================
    Index helpers (optional placeholders)
    =========================
*/

int table_find_row(Table *table, int row_id)
{
    if (!table || !table->row_ids) {
        return -1;
    }

    for (int i = 0; i < table->row_count; i++) {
        if (table->row_ids[i] == row_id) {
            return i;
        }
    }

    return -1;
}

int table_find_column(Table *table, const char *name)
{
    if (!table || !name || !table->col_names) {
        return -1;
    }

    for (int i = 0; i < table->col_count; i++) {
        if (strcmp(table->col_names[i], name) == 0) {
            return i;
        }
    }

    return -1;
}

/*
    =========================
    CSV output (debug print)
    =========================
*/

void table_print_csv(const Table *table)
{
    if (!table || !table->cells) {
        return;
    }

    // header (если есть имена колонок)
    if (table->col_names) {
        printf(",");
        for (int c = 0; c < table->col_count; c++) {
            printf("%s", table->col_names[c]);
            if (c < table->col_count - 1) {
                printf(",");
            }
        }
        printf("\n");
    }

    for (int r = 0; r < table->row_count; r++) {

        if (table->row_ids) {
            printf("%d,", table->row_ids[r]);
        } else {
            if(r == 0) {
                printf(",");
            } else {
            printf("%d,", r);
            }
        }

        for (int c = 0; c < table->col_count; c++) {

            const Cell *cell = &table->cells[r][c];

            if (cell->type == CELL_NUMBER) {
                printf("%d", cell->data.number);
            }
            else if (cell->type == CELL_FORMULA) {
                // пока не вычислено evaluator'ом
                printf("=FORMULA");
            }
            else {
                printf("0");
            }

            if (c < table->col_count - 1) {
                printf(",");
            }
        }

        printf("\n");
    }
}