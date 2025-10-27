#ifndef MARKDOWN_H
#define MARKDOWN_H

#include <stdio.h>

/**
 * @brief An opaque struct representing an open Markdown file.
 */
typedef struct MarkdownHandle MarkdownHandle;

/**
 * @brief Opens a new Markdown file for writing.
 *
 * @param filename The path to the file to open (will be overwritten).
 * @return A pointer to a new MarkdownHandle, or NULL on failure.
 */
MarkdownHandle *md_open_file(const char *filename);

/**
 * @brief Closes the Markdown file and frees the handle.
 *
 * @param handle The handle to close.
 */
void md_close_file(MarkdownHandle *handle);

/**
 * @brief Adds a header to the Markdown file.
 *
 * @param handle The Markdown file handle.
 * @param level The header level (1 for #, 2 for ##, etc.).
 * @param text The header text.
 */
void md_add_header(MarkdownHandle *handle, int level, const char *text);

/**
 * @brief Adds a fenced code block to the Markdown file.
 *
 * @param handle The Markdown file handle.
 * @param language_tag The syntax highlighting tag (e.g., "c", "python").
 * @param content The code content to write inside the block.
 */
void md_add_code_block(MarkdownHandle *handle, const char *language_tag, const char *content);

/**
 * @brief Adds raw text (verbatim) to the Markdown file.
 *
 * @param handle The Markdown file handle.
 * @param text The raw text to append.
 */
void md_add_raw_text(MarkdownHandle *handle, const char *text);

#endif // MARKDOWN_H