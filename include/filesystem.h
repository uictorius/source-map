#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "config.h"
#include "markdown.h"

/**
 * @brief Generates a directory tree and appends it to the Markdown file.
 *
 * Tries to use the system 'tree' command first, respecting .gitignore.
 * Falls back to a native C implementation if 'tree' is not available.
 *
 * @param md The Markdown file handle.
 * @param root_path The root directory of the project to scan.
 * @param output_file The name of the final .md file (to be ignored).
 */
void generate_directory_tree(MarkdownHandle *md, const char *root_path, const char *output_file);

/**
 * @brief Scans all project files and appends their content to the Markdown file.
 *
 * Traverses the directory, respects .gitignore, and uses the language
 * profile to filter which files to include.
 *
 * @param md The Markdown file handle.
 * @param root_path The root directory of the project to scan.
 * @param profile The language profile defining filter rules.
 * @param output_file The name of the final .md file (to be ignored).
 */
void process_project_files(MarkdownHandle *md, const char *root_path,
                           const LanguageProfile *profile, const char *output_file);

#endif // FILESYSTEM_H