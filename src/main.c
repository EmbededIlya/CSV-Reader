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
    
    printf("CSV-Reader started with file: %s\n", argv[1]);
    return 0;
}