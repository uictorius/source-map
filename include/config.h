#ifndef CONFIG_H
#define CONFIG_H

#define MAX_EXTENSIONS 50
#define MAX_FILENAMES 50
#define MAX_STR_LEN 100

/**
 * @brief Holds all configuration settings for a specific language profile.
 *
 * This structure is populated from an .ini file and defines which files
 * to include, which to ignore, and how to map them to syntax highlighting.
 */
typedef struct {
    char language_name[MAX_STR_LEN];
    char *allowed_extensions[MAX_EXTENSIONS];
    char *allowed_dotfiles[MAX_FILENAMES];
    char *allowed_filenames[MAX_FILENAMES];
    char *ignored_extensions[MAX_EXTENSIONS];
    char *ignored_filenames[MAX_FILENAMES];
    struct {
        char *ext; // The file extension or name (e.g., "c" or "Makefile")
        char *tag; // The markdown syntax tag (e.g., "c" or "makefile")
    } syntax_map[MAX_EXTENSIONS];
    int allowed_extensions_count;
    int allowed_dotfiles_count;
    int allowed_filenames_count;
    int ignored_extensions_count;
    int ignored_filenames_count;
    int syntax_map_count;
} LanguageProfile;

/**
 * @brief Loads a language profile from a corresponding .ini file.
 *
 * Searches in standard paths (./config, ~/.config/source-map, etc.)
 * for a file named "<language>.ini".
 *
 * @param language The name of the language profile to load (e.g., "c").
 * @return A pointer to a new LanguageProfile struct, or NULL on failure.
 * The caller is responsible for freeing this memory with
 * free_language_profile().
 */
LanguageProfile *load_language_profile(const char *language);

/**
 * @brief Frees all memory associated with a LanguageProfile struct.
 *
 * @param profile The profile to free.
 */
void free_language_profile(LanguageProfile *profile);

/**
 * @brief Gets the correct Markdown syntax tag for a given filename.
 *
 * It checks the syntax_map for a match first by full filename (for
 * files like "Makefile") and then by extension.
 *
 * @param profile The language profile.
 * @param filename The simple filename (e.g., "main.c" or "Makefile").
 * @return The corresponding syntax tag (e.g., "c" or "makefile").
 * Defaults to "txt" or the extension itself if no map is found.
 */
const char *get_syntax_tag(const LanguageProfile *profile, const char *filename);

#endif // CONFIG_H