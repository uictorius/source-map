#ifndef GITIGNORE_H
#define GITIGNORE_H

#include <stdbool.h>

/**
 * @brief An opaque struct holding compiled gitignore patterns.
 */
typedef struct Gitignore Gitignore;

/**
 * @brief Loads and parses .gitignore rules from a base path.
 *
 * @param base_path The directory containing the .gitignore file.
 * @return A pointer to a new Gitignore struct, or NULL on failure.
 * The caller is responsible for freeing this memory with
 * gitignore_free().
 */
Gitignore *gitignore_load(const char *base_path);

/**
 * @brief Frees all memory associated with a Gitignore struct.
 *
 * @param gi The Gitignore struct to free.
 */
void gitignore_free(Gitignore *gi);

/**
 * @brief Checks if a given path matches any .gitignore rules.
 *
 * @param gi The loaded Gitignore struct.
 * @param path The relative path to check (e.g., "src/main.c").
 * @param is_dir Whether the path is a directory.
 * @return true if the path is ignored, false otherwise.
 */
bool gitignore_matches_path(const Gitignore *gi, const char *path, bool is_dir);

#endif // GITIGNORE_H