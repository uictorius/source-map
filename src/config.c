#define _GNU_SOURCE // For strdup() and wordexp()
#include "config.h"
#include "iniparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h> // Added for tilde expansion

#define MAX_PATHS 3
#define PATH_BUF_SIZE 1024

// Dummy variable, for type-checking in some environments.
LanguageProfile *dummy = NULL;

/**
 * @brief Parses a comma-separated string into an array of strings.
 *
 * @param str The string to parse (e.g., "c,h,md"). This string is modified.
 * @param arr The array to store the resulting string pointers.
 * @param max_count The maximum number of items to store in arr.
 * @return The number of items parsed.
 */
static int parse_comma_separated_string(char *str, char *arr[], int max_count)
{
    if (!str)
        return 0;
    int count = 0;
    char *token = strtok(str, ",");
    while (token && count < max_count) {
        arr[count++] = strdup(token);
        token = strtok(NULL, ",");
    }
    return count;
}

/**
 * @brief Parses a comma-separated "key:value" string into the syntax_map.
 *
 * @param str The string to parse (e.g., "c:c,h:c,Makefile:makefile").
 * @param profile The profile where the map will be stored.
 * @param max_count The maximum number of items to store.
 * @return The number of key:value pairs parsed.
 */
static int parse_syntax_map(char *str, LanguageProfile *profile, int max_count)
{
    if (!str || !profile)
        return 0;
    int count = 0;
    char *token = strtok(str, ",");
    while (token && count < max_count) {
        char *colon = strchr(token, ':');
        if (colon) {
            *colon = '\0';
            profile->syntax_map[count].ext = strdup(token);
            profile->syntax_map[count].tag = strdup(colon + 1);
            count++;
        }
        token = strtok(NULL, ",");
    }
    return count;
}

/**
 * @brief Helper function to expand tilde (~) paths.
 *
 * @param path The path string to expand (e.g., "~/.config").
 * @return A new, allocated string with the expanded path.
 * The caller must free the returned string.
 */
static char *expand_path(const char *path)
{
    wordexp_t p;
    char *expanded = NULL;

    if (wordexp(path, &p, 0) == 0) {
        if (p.we_wordc > 0) {
            expanded = strdup(p.we_wordv[0]);
        }
        wordfree(&p);
    }
    return expanded;
}

LanguageProfile *load_language_profile(const char *language)
{
    dictionary *ini = NULL;
    char ini_path[PATH_BUF_SIZE];

    // Standard search paths for config files
    const char *config_paths[MAX_PATHS] = {"./config", "~/.config/source-map",
                                           "/usr/local/share/source-map/config"};

    for (int i = 0; i < MAX_PATHS; i++) {
        char *base_path = NULL;
        if (config_paths[i][0] == '~') {
            base_path = expand_path(config_paths[i]);
        }
        else {
            base_path = strdup(config_paths[i]);
        }

        if (!base_path)
            continue;

        snprintf(ini_path, sizeof(ini_path), "%s/%s.ini", base_path, language);
        free(base_path); // Free the expanded/duplicated path

        if (access(ini_path, F_OK) == 0) {
            ini = iniparser_load(ini_path);
            if (ini)
                break; // File found and loaded
        }
    }

    if (!ini) {
        fprintf(stderr, "Error: Could not load language profile '%s'.\n", language);
        return NULL;
    }

    LanguageProfile *profile = calloc(1, sizeof(LanguageProfile));
    if (!profile) {
        iniparser_freedict(ini);
        return NULL;
    }

    // Load all filter strings from the .ini file
    char *allowed_ext_str = strdup(iniparser_getstring(ini, "Filters:allowed_extensions", ""));
    char *allowed_dot_str = strdup(iniparser_getstring(ini, "Filters:allowed_dotfiles", ""));
    char *allowed_file_str = strdup(iniparser_getstring(ini, "Filters:allowed_filenames", ""));
    char *ignored_ext_str = strdup(iniparser_getstring(ini, "Filters:ignored_extensions", ""));
    char *ignored_file_str = strdup(iniparser_getstring(ini, "Filters:ignored_filenames", ""));
    char *syntax_map_str = strdup(iniparser_getstring(ini, "Markdown:syntax_map", ""));

    strncpy(profile->language_name, iniparser_getstring(ini, "Core:language_name", "Project"),
            MAX_STR_LEN - 1);
    profile->language_name[MAX_STR_LEN - 1] = '\0'; // Ensure null termination

    // Parse the comma-separated strings into the profile struct
    profile->allowed_extensions_count =
        parse_comma_separated_string(allowed_ext_str, profile->allowed_extensions, MAX_EXTENSIONS);
    profile->allowed_dotfiles_count =
        parse_comma_separated_string(allowed_dot_str, profile->allowed_dotfiles, MAX_FILENAMES);
    profile->allowed_filenames_count =
        parse_comma_separated_string(allowed_file_str, profile->allowed_filenames, MAX_FILENAMES);
    profile->ignored_extensions_count =
        parse_comma_separated_string(ignored_ext_str, profile->ignored_extensions, MAX_EXTENSIONS);
    profile->ignored_filenames_count =
        parse_comma_separated_string(ignored_file_str, profile->ignored_filenames, MAX_FILENAMES);
    profile->syntax_map_count = parse_syntax_map(syntax_map_str, profile, MAX_EXTENSIONS);

    // Free the temporary strings
    free(allowed_ext_str);
    free(allowed_dot_str);
    free(allowed_file_str);
    free(ignored_ext_str);
    free(ignored_file_str);
    free(syntax_map_str);

    iniparser_freedict(ini);
    return profile;
}

void free_language_profile(LanguageProfile *profile)
{
    if (!profile)
        return;

    // Free all dynamically allocated strings inside the profile
    for (int i = 0; i < profile->allowed_extensions_count; i++)
        free(profile->allowed_extensions[i]);
    for (int i = 0; i < profile->allowed_dotfiles_count; i++)
        free(profile->allowed_dotfiles[i]);
    for (int i = 0; i < profile->allowed_filenames_count; i++)
        free(profile->allowed_filenames[i]);
    for (int i = 0; i < profile->ignored_extensions_count; i++)
        free(profile->ignored_extensions[i]);
    for (int i = 0; i < profile->ignored_filenames_count; i++)
        free(profile->ignored_filenames[i]);
    for (int i = 0; i < profile->syntax_map_count; i++) {
        free(profile->syntax_map[i].ext);
        free(profile->syntax_map[i].tag);
    }

    // Free the profile itself
    free(profile);
}

const char *get_syntax_tag(const LanguageProfile *profile, const char *filename)
{
    if (!profile || !filename)
        return "txt";

    // Get the extension (if any)
    const char *ext = strrchr(filename, '.');
    ext = (ext && ext != filename) ? ext + 1 : "";

    for (int i = 0; i < profile->syntax_map_count; i++) {
        // Try to match by full filename first (e.g., "Makefile")
        if (strcmp(profile->syntax_map[i].ext, filename) == 0) {
            return profile->syntax_map[i].tag;
        }
        // Try to match by extension (e.g., "c")
        if (ext[0] != '\0' && strcmp(profile->syntax_map[i].ext, ext) == 0) {
            return profile->syntax_map[i].tag;
        }
    }

    // If an extension exists but wasn't mapped, use the extension itself as the tag
    if (ext[0] != '\0') {
        return ext;
    }

    // Final fallback
    return "txt";
}