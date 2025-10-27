#ifndef INIPARSER_H
#define INIPARSER_H

/**
 * @brief An opaque struct holding the key/value pairs from an .ini file.
 */
typedef struct dictionary dictionary;

/**
 * @brief Loads and parses an .ini file into a dictionary.
 *
 * Supports [Section] headers and "key = value" pairs.
 * Keys are stored internally as "Section:key".
 *
 * @param filename The path to the .ini file.
 * @return A pointer to a new dictionary struct, or NULL on failure.
 * The caller is responsible for freeing this memory with
 * iniparser_freedict().
 */
dictionary *iniparser_load(const char *filename);

/**
 * @brief Frees all memory associated with a dictionary struct.
 *
 * @param d The dictionary to free.
 */
void iniparser_freedict(dictionary *d);

/**
 * @brief Retrieves a string value from the dictionary by key.
 *
 * @param d The dictionary.
 * @param key The key to search for (e.g., "Section:key").
 * @param def A default value to return if the key is not found.
 * @return The corresponding value string, or the default value.
 */
const char *iniparser_getstring(dictionary *d, const char *key, const char *def);

#endif // INIPARSER_H