#include "config.h"
#include "filesystem.h"
#include "markdown.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Prints the command-line usage instructions.
 * @param prog_name The name of the executable (argv[0]).
 */
void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <language_profile> [target_directory] [output_file]\n", prog_name);
}

/**
 * @brief Main entry point for the source-map utility.
 */
int main(int argc, char *argv[])
{
    // --- Argument Parsing ---
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *language = argv[1];
    const char *target_dir = (argc > 2) ? argv[2] : ".";
    const char *output_file = (argc > 3) ? argv[3] : "output.md";

    // --- Profile Loading ---
    LanguageProfile *profile = load_language_profile(language);
    if (!profile) {
        return 1; // Error message already printed by load_language_profile
    }

    // --- Markdown File Init ---
    MarkdownHandle *md = md_open_file(output_file);
    if (!md) {
        fprintf(stderr, "Error: Could not open output file '%s'.\n", output_file);
        free_language_profile(profile);
        return 1;
    }

    // --- Report Generation ---
    md_add_header(md, 1, profile->language_name);

    // 1. Directory Tree
    md_add_header(md, 2, "Directory Tree");
    generate_directory_tree(md, target_dir, output_file);

    // 2. File Contents
    md_add_header(md, 2, "File Contents");
    process_project_files(md, target_dir, profile, output_file);

    // --- Cleanup ---
    md_close_file(md);
    free_language_profile(profile);

    printf("Export complete: %s\n", output_file);
    return 0;
}