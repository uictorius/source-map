#include "markdown.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal representation of a Markdown file handle.
 */
struct MarkdownHandle {
    FILE *file;
};

MarkdownHandle *md_open_file(const char *filename)
{
    MarkdownHandle *handle = malloc(sizeof(MarkdownHandle));
    if (!handle)
        return NULL;

    handle->file = fopen(filename, "w");
    if (!handle->file) {
        free(handle);
        return NULL;
    }
    return handle;
}

void md_close_file(MarkdownHandle *handle)
{
    if (handle) {
        if (handle->file) {
            fclose(handle->file);
        }
        free(handle);
    }
}

void md_add_header(MarkdownHandle *handle, int level, const char *text)
{
    if (!handle || !handle->file)
        return;
    for (int i = 0; i < level; i++) {
        fputc('#', handle->file);
    }
    fprintf(handle->file, " %s\n\n", text);
}

void md_add_code_block(MarkdownHandle *handle, const char *language_tag, const char *content)
{
    if (!handle || !handle->file)
        return;
    fprintf(handle->file, "```%s\n", language_tag ? language_tag : "");
    fprintf(handle->file, "%s\n", content);
    fprintf(handle->file, "```\n\n");
}

void md_add_raw_text(MarkdownHandle *handle, const char *text)
{
    if (!handle || !handle->file)
        return;
    // Use fprintf for raw text to handle potential '%' characters safely
    fprintf(handle->file, "%s", text);
}