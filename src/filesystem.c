#define _POSIX_C_SOURCE 200809L
#include "filesystem.h"
#include "gitignore.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PATH_MAX_LEN 4096

/**
 * @brief Native fallback if 'tree' command is not available.
 *
 * Recursively scans a directory and prints its structure to the Markdown file.
 *
 * @param md The Markdown file handle.
 * @param base_path The current directory being scanned.
 * @param indent_level The current depth for indentation.
 */
static void native_tree_fallback(MarkdownHandle *md, const char *base_path, int indent_level)
{
    DIR *dir = opendir(base_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[PATH_MAX_LEN];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        char indent_str[PATH_MAX_LEN];
        size_t indent_len = (size_t)indent_level * 4; // 4 spaces per indent level

        // Ensure indent string doesn't overflow buffer
        if (indent_len >= sizeof(indent_str)) {
            indent_len = sizeof(indent_str) - 1;
        }
        memset(indent_str, ' ', indent_len);
        indent_str[indent_len] = '\0';

        char line[PATH_MAX_LEN];

        // Safe concatenation to build the line (e.g., "    |-- main.c\n")
        strncpy(line, indent_str, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0'; // Ensure null-termination

        strncat(line, "|-- ", sizeof(line) - strlen(line) - 1);
        strncat(line, entry->d_name, sizeof(line) - strlen(line) - 1);
        strncat(line, "\n", sizeof(line) - strlen(line) - 1);

        md_add_raw_text(md, line);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
            native_tree_fallback(md, path, indent_level + 1);
    }
    closedir(dir);
}

void generate_directory_tree(MarkdownHandle *md, const char *root_path, const char *output_file)
{
    char command[PATH_MAX_LEN];
    // Use 'tree' if available, as it respects .gitignore and looks better
    snprintf(command, sizeof(command), "tree --gitignore -a -I \"%s|.git\" \"%s\"", output_file,
             root_path);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        // Fallback if 'tree' command fails or is not installed
        fprintf(stderr, "Warning: 'tree' command not found. Using native fallback.\n");
        md_add_raw_text(md, "```\n");
        md_add_raw_text(md, root_path);
        md_add_raw_text(md, "\n");
        native_tree_fallback(md, root_path, 0);
        md_add_raw_text(md, "```\n");
        return;
    }

    char buffer[PATH_MAX_LEN];
    md_add_raw_text(md, "```\n");
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        md_add_raw_text(md, buffer);
    md_add_raw_text(md, "```\n");
    pclose(pipe);
}

/**
 * @brief Checks if a file should be included based on the language profile.
 *
 * @param path The relative path to the file.
 * @param profile The loaded language profile with filter rules.
 * @return true if the file is allowed, false otherwise.
 */
static bool is_file_allowed(const char *path, const LanguageProfile *profile)
{
    const char *filename = strrchr(path, '/');
    filename = filename ? filename + 1 : path;

    const char *ext = strrchr(filename, '.');
    ext = (ext && ext != filename) ? ext + 1 : "";

    // Check ignore lists first
    for (int i = 0; i < profile->ignored_filenames_count; i++)
        if (strcmp(filename, profile->ignored_filenames[i]) == 0)
            return false;

    for (int i = 0; i < profile->ignored_extensions_count; i++)
        if (strcmp(ext, profile->ignored_extensions[i]) == 0)
            return false;

    // Check allow lists
    if (filename[0] == '.') {
        for (int i = 0; i < profile->allowed_dotfiles_count; i++)
            if (strcmp(filename, profile->allowed_dotfiles[i]) == 0)
                return true;
    }

    for (int i = 0; i < profile->allowed_filenames_count; i++)
        if (strcmp(filename, profile->allowed_filenames[i]) == 0)
            return true;

    for (int i = 0; i < profile->allowed_extensions_count; i++)
        if (strcmp(ext, profile->allowed_extensions[i]) == 0)
            return true;

    // Default to deny
    return false;
}

/**
 * @brief Recursively traverses the directory and processes allowed files.
 *
 * @param md The Markdown file handle.
 * @param base_path The current directory being scanned.
 * @param profile The language profile with filter rules.
 * @param gi The loaded .gitignore rules.
 * @param output_file The name of the final .md file (to be ignored).
 */
static void traverse_and_process(MarkdownHandle *md, const char *base_path,
                                 const LanguageProfile *profile, Gitignore *gi,
                                 const char *output_file)
{
    DIR *dir = opendir(base_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[PATH_MAX_LEN];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) != 0)
            continue;

        bool is_dir = S_ISDIR(statbuf.st_mode);

        // Check .gitignore rules
        if (gitignore_matches_path(gi, path, is_dir))
            continue;

        if (is_dir) {
            // Recurse into subdirectory
            traverse_and_process(md, path, profile, gi, output_file);
        }
        else {
            const char *filename = strrchr(path, '/');
            filename = filename ? filename + 1 : path;

            // Don't include the output file itself
            if (strcmp(filename, output_file) == 0)
                continue;

            // Check if the file is allowed by the profile
            if (is_file_allowed(path, profile)) {
                md_add_header(md, 3, path); // Add file path as a header
                FILE *file = fopen(path, "rb");
                if (file) {
                    fseek(file, 0, SEEK_END);
                    long length = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char *content = malloc(length + 1);
                    if (content) {
                        fread(content, 1, length, file);
                        content[length] = '\0';

                        const char *tag = get_syntax_tag(profile, filename);
                        md_add_code_block(md, tag, content);
                        free(content);
                    }
                    fclose(file);
                }
            }
        }
    }
    closedir(dir);
}

void process_project_files(MarkdownHandle *md, const char *root_path,
                           const LanguageProfile *profile, const char *output_file)
{
    Gitignore *gi = gitignore_load(root_path);
    traverse_and_process(md, root_path, profile, gi, output_file);
    gitignore_free(gi);
}