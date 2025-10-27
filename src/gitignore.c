#define _GNU_SOURCE // For strdup()
#include "gitignore.h"

#include <fnmatch.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATTERNS 512
#define MAX_PATTERN_LEN 256

/**
 * @brief Internal representation of gitignore rules.
 */
struct Gitignore {
    char *patterns[MAX_PATTERNS];
    bool is_negation[MAX_PATTERNS]; // true if the pattern is an exclusion (starts with !)
    int count;
};

/**
 * @brief Loads patterns from a single .gitignore file.
 *
 * @param gi The Gitignore struct to populate.
 * @param filepath Path to the .gitignore file.
 */
static void load_patterns_from_file(Gitignore *gi, const char *filepath)
{
    FILE *file = fopen(filepath, "r");
    if (!file)
        return;

    char line[MAX_PATTERN_LEN];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0; // Remove newline

        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\0')
            continue;

        if (gi->count >= MAX_PATTERNS)
            break; // Stop if we've read too many patterns

        char *pattern = line;
        if (pattern[0] == '!') {
            gi->is_negation[gi->count] = true;
            pattern++; // Skip the '!'
        }
        else {
            gi->is_negation[gi->count] = false;
        }

        gi->patterns[gi->count] = strdup(pattern);
        gi->count++;
    }
    fclose(file);
}

Gitignore *gitignore_load(const char *base_path)
{
    Gitignore *gi = calloc(1, sizeof(Gitignore));
    if (!gi)
        return NULL;

    char gitignore_path[1024];
    snprintf(gitignore_path, sizeof(gitignore_path), "%s/.gitignore", base_path);

    load_patterns_from_file(gi, gitignore_path);
    // Note: A full implementation would search parent directories.
    // This implementation only loads .gitignore from the root.
    return gi;
}

void gitignore_free(Gitignore *gi)
{
    if (!gi)
        return;
    for (int i = 0; i < gi->count; i++)
        free(gi->patterns[i]);
    free(gi);
}

bool gitignore_matches_path(const Gitignore *gi, const char *path, bool is_dir)
{
    if (!gi)
        return false;

    bool is_ignored = false;
    const char *relative_path = path;

    // Normalize path (remove leading ./)
    if (strncmp(path, "./", 2) == 0)
        relative_path += 2;

    // Check all patterns in order
    for (int i = 0; i < gi->count; i++) {
        char pattern[MAX_PATTERN_LEN];
        strncpy(pattern, gi->patterns[i], sizeof(pattern) - 1);
        pattern[sizeof(pattern) - 1] = '\0';

        // Handle directory-only patterns (e.g., "build/")
        size_t len = strlen(pattern);
        if (len > 0 && pattern[len - 1] == '/') {
            if (!is_dir)
                continue;            // This rule only applies to directories
            pattern[len - 1] = '\0'; // Remove trailing slash for matching
        }

        // Use fnmatch to check for wildcard matches
        if (fnmatch(pattern, relative_path, FNM_PATHNAME) == 0)
            is_ignored = !gi->is_negation[i]; // Last match wins
    }

    return is_ignored;
}