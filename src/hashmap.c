/**
 * @file hashmap.c
 * @brief Simple hashmap implementation for CSV cell lookup.
 *
 * This module maps string keys ("A1", "Cell30") to cell coordinates.
 * Uses open addressing with linear probing.
 */

#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * @brief Internal hash map entry.
 */
typedef struct {
    char key[64];
    int row;
    int col;
    int used;
} Entry;

/**
 * @brief Internal hashmap structure.
 */
struct HashMap {
    Entry *entries;
    int capacity;
};

/* --- Internal helpers --- */

/**
 * @brief Simple hash function (djb2).
 */
static unsigned long hash_str(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

/**
 * @brief Creates hashmap.
 */
HashMap* hashmap_create(int capacity)
{
    if (capacity <= 0) {
        return NULL;
    }

    HashMap *map = (HashMap *)calloc(1, sizeof(HashMap));
    if (!map) {
        return NULL;
    }

    map->capacity = capacity;

    map->entries = (Entry *)calloc(capacity, sizeof(Entry));
    if (!map->entries) {
        free(map);
        return NULL;
    }

    return map;
}

/**
 * @brief Destroys hashmap.
 */
void hashmap_destroy(HashMap *map)
{
    if (!map) {
        return;
    }

    free(map->entries);
    free(map);
}

/**
 * @brief Inserts key -> (row,col).
 */
int hashmap_put(HashMap *map, const char *key, int row, int col)
{
    if (!map || !key) {
        return 0;
    }

    unsigned long hash = hash_str(key);
    int idx = hash % map->capacity;

    for (int i = 0; i < map->capacity; i++) {
        int pos = (idx + i) % map->capacity;

        if (!map->entries[pos].used ||
            strcmp(map->entries[pos].key, key) == 0)
        {
            strncpy(map->entries[pos].key, key, 63);
            map->entries[pos].key[63] = '\0';

            map->entries[pos].row = row;
            map->entries[pos].col = col;
            map->entries[pos].used = 1;

            return 1;
        }
    }

    return 0;
}

/**
 * @brief Gets (row,col) by key.
 */
int hashmap_get(HashMap *map, const char *key, int *row, int *col)
{
    if (!map || !key || !row || !col) {
        return 0;
    }

    unsigned long hash = hash_str(key);
    int idx = hash % map->capacity;

    for (int i = 0; i < map->capacity; i++) {
        int pos = (idx + i) % map->capacity;

        if (!map->entries[pos].used) {
            return 0;
        }

        if (strcmp(map->entries[pos].key, key) == 0) {
            *row = map->entries[pos].row;
            *col = map->entries[pos].col;
            return 1;
        }
    }

    return 0;
}