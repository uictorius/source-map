#define _GNU_SOURCE // For strdup()
#include "iniparser.h"
#include <ctype.h> // Added for isspace()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRIES 512
#define MAX_LINE_LEN 1024

/**
 * @brief Internal dictionary structure to hold .ini file entries.
 */
struct dictionary {
    char *key[MAX_ENTRIES]; // Stores keys as "Section:key"
    char *val[MAX_ENTRIES]; // Stores values
    int count;              // Number of entries
};

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * Modifies the string in place by moving the start pointer and adding
 * a new null-terminator.
 *
 * @param str The string to trim.
 * @return A pointer to the *new* start of the trimmed string.
 */
static char *trim_whitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All-whitespace string
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator
    *(end + 1) = '\0';

    return str;
}

/**
 * @brief Checks for and skips a UTF-8 BOM (Byte Order Mark) at the
 * beginning of a file.
 *
 * @param file An open FILE pointer.
 */
static void skip_utf8_bom(FILE *file)
{
    unsigned char bom[3];
    if (fread(bom, 1, 3, file) == 3) {
        // Check for {0xEF, 0xBB, 0xBF}
        if (bom[0] != 0xEF || bom[1] != 0xBB || bom[2] != 0xBF) {
            // Not a BOM, rewind to the beginning
            fseek(file, 0, SEEK_SET);
        }
        // If it was a BOM, the file pointer is now correctly positioned
    }
    else {
        // File too short or read error, rewind
        fseek(file, 0, SEEK_SET);
    }
}

dictionary *iniparser_load(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return NULL;

    // Handle potential UTF-8 BOM, which can break parsing
    skip_utf8_bom(file);

    dictionary *d = calloc(1, sizeof(dictionary));
    if (!d) {
        fclose(file);
        return NULL;
    }

    char line_buffer[MAX_LINE_LEN];
    char section[MAX_LINE_LEN] = "";

    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        line_buffer[strcspn(line_buffer, "\r\n")] = 0; // Remove newline
        char *line = line_buffer;

        // Skip empty lines and comments
        line = trim_whitespace(line);
        if (line[0] == ';' || line[0] == '#' || line[0] == '\0')
            continue;

        // Section line: [SectionName]
        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            strncpy(section, line + 1, strlen(line) - 2);
            section[strlen(line) - 2] = '\0';
            trim_whitespace(section); // Trim whitespace from section name
            continue;
        }

        // Key/Value line: Key = Value
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *key = trim_whitespace(line);
            char *val = trim_whitespace(eq + 1);

            if (d->count < MAX_ENTRIES) {
                char full_key[MAX_LINE_LEN];

                // Safely construct the full key: "Section:Key"
                strncpy(full_key, section, sizeof(full_key) - 1);
                full_key[sizeof(full_key) - 1] = '\0';

                strncat(full_key, ":", sizeof(full_key) - strlen(full_key) - 1);
                strncat(full_key, key, sizeof(full_key) - strlen(full_key) - 1);

                d->key[d->count] = strdup(full_key);
                d->val[d->count] = strdup(val);
                d->count++;
            }
        }
    }
    fclose(file);
    return d;
}

void iniparser_freedict(dictionary *d)
{
    if (!d)
        return;
    // Free all duplicated key and value strings
    for (int i = 0; i < d->count; i++) {
        free(d->key[i]);
        free(d->val[i]);
    }
    // Free the dictionary itself
    free(d);
}

const char *iniparser_getstring(dictionary *d, const char *key, const char *def)
{
    if (!d || !key)
        return def;
    for (int i = 0; i < d->count; i++) {
        if (strcmp(d->key[i], key) == 0) {
            return d->val[i];
        }
    }
    return def;
}