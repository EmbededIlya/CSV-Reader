/**
 * @file hashmap.h
 * @brief Lightweight hash map for fast cell lookup in CSV Table module.
 *
 * Provides O(1)-average access to table cells using string keys
 * (e.g. "A:1", "B:3"). Implemented using open addressing with
 * linear probing.
 *
 * This module is designed as an internal support structure for the
 * Table API and is not intended for direct external manipulation.
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>

/**
 * @brief Public representation of a hash map entry.
 *
 * Stores mapping between a string key and a table cell coordinate.
 * This structure is primarily used internally and exposed only for
 * debugging or inspection purposes.
 */
typedef struct {
    char key[64]; /**< Null-terminated string key (e.g. "A:1") */
    int row;      /**< Row index in the table */
    int col;      /**< Column index in the table */
} HashEntry;

/**
 * @brief Opaque HashMap structure (implementation hidden).
 *
 * This enforces encapsulation using the Pimpl (Pointer to implementation)
 * pattern, preventing direct access to internal storage.
 */
typedef struct HashMap HashMap;

/* --- Lifecycle Operations --- */

/**
 * @brief Creates a new hash map instance.
 *
 * Allocates memory for internal storage based on the given capacity.
 *
 * @param capacity Maximum number of entries the map can hold.
 * @return HashMap* Pointer to created instance, or NULL on failure.
 */
HashMap* hashmap_create(int capacity);

/**
 * @brief Destroys a hash map and releases all allocated memory.
 *
 * After calling this function, the pointer becomes invalid.
 *
 * @param map Pointer to HashMap instance. Safe to pass NULL.
 */
void hashmap_destroy(HashMap *map);

/* --- Data Operations --- */

/**
 * @brief Inserts or updates a key-value mapping.
 *
 * If the key already exists, its value is overwritten.
 *
 * @param map   Pointer to HashMap instance.
 * @param key   Null-terminated string key (max 63 chars + '\0').
 * @param row   Row index associated with the key.
 * @param col   Column index associated with the key.
 * @return int  1 on success, 0 on failure (e.g. map full or NULL input).
 */
int hashmap_put(HashMap *map, const char *key, int row, int col);

/**
 * @brief Retrieves a value by key.
 *
 * Performs linear probing lookup using the internal hash function.
 *
 * @param map   Pointer to HashMap instance.
 * @param key   Null-terminated string key to search for.
 * @param row   Output pointer for row index.
 * @param col   Output pointer for column index.
 * @return int  1 if key was found, 0 otherwise.
 */
int hashmap_get(HashMap *map, const char *key, int *row, int *col);

#endif /* HASHMAP_H */