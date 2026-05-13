#include <stdio.h>
#include "table.h"
#include "parser.h"
#include "error.h"
#include "evaluator.h"
#include "csv.h"


int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: csvreader <file.csv>\n");
        return 1;
    }


    Table *table = table_create(3, 3);
    if (!table) {
        fprintf(stderr, "Failed to create table\n");
        return 1;
    }
    table->cells[0][0].type = CELL_NUMBER;
    table->cells[0][0].data.number = 42;

    table_print_csv(table);
    table_destroy(table);


    printf("CSV-Reader started with file: %s\n", argv[1]);
    return 0;
}