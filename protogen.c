// Protogen prototype generator

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    size_t len;
    size_t cap;
    char *data;
} str_t;

void str_new(str_t *ref_str);
void str_push(str_t *ref_str, char c);
void str_append(str_t *ref_str, char *data);
void str_free(str_t *ref_str);

int gen_file_headers(
    const char *const fname,
    str_t *ref_g_hdr, str_t *ref_l_hdr
);

int save_file(const char *const fname, const char *const data);
char *basename(const char *const path);

bool is_wspace(char c);

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(
            stderr,
            "Error: Expected an output directory and at least one source!\n"
        );
        printf("protogen by Dylan Turner\n");
        printf("Usage: protogen OUTDIR SRCS...\n");
        return 1;
    }
    char *out_dir = argv[1];
    char **srcs = argv + 2;
    size_t n_srcs = argc - 2;
    str_t global_header = { 0 };
    str_new(&global_header);
    str_append(
        &global_header,
        "#pragma once\n\n#define fn \n#define pub \n\n"
    );
    str_t local_header = { 0 };
    char fname[FILENAME_MAX] = "";
    for (int i = 0; i < n_srcs; i++) {
        str_new(&local_header);
        str_append(&local_header, "#pragma once\n\n");
        printf("Creating function prototypes for %s...\n", argv[i + 2]);
        if (!gen_file_headers(srcs[i], &global_header, &local_header)) {
            fprintf(stderr, "Couldn't extract fns from %s!\n", fname);
        } else {
            printf("Header created!\n");
        }
        char *base = basename(srcs[i]);
        sprintf(fname, "%s/%s_fns.h", out_dir, base);
        printf("Saving to %s...\n", fname);
        if (!save_file(fname, local_header.data)) {
            fprintf(stderr,"Failed to save %s...\n", fname);
        } else {
            printf("Saved!\n");
        }
        str_free(&local_header);
    }
    sprintf(fname, "%s/fns.h", out_dir);
    printf("Saving globals to %s...\n", fname);
    fflush(NULL);
    if (!save_file(fname, global_header.data)) {
        fprintf(stderr, "Failed to save %s...\n", fname);
        fflush(NULL);
    }
    str_free(&global_header);
    return 0;
}

void str_new(str_t *ref_str) {
    ref_str->cap = 1;
    ref_str->len = ref_str->cap - 1;
    ref_str->data = NULL;
    ref_str->data = malloc(ref_str->cap);
    ref_str->data[0] = '\0';
}

void str_push(str_t *ref_str, char c) {
    if (ref_str->len + 2 > ref_str->cap) {
        // Resize
        ref_str->cap = (ref_str->len + 1) * 2;
        ref_str->data = realloc(ref_str->data, ref_str->cap);
    }
    ref_str->data[ref_str->len] = c;
    ref_str->len++;
    ref_str->data[ref_str->len] = '\0';
}

void str_append(str_t *ref_str, char *data) {
    size_t new_data_len = strlen(data);
    if (ref_str->len + new_data_len + 1 > ref_str->cap) {
        // Resize
        if (new_data_len + 1 < (ref_str->len + 1) * 2) {
            // < double → double
            ref_str->cap = (ref_str->len + 1) * 2;
            ref_str->data = realloc(ref_str->data, ref_str->cap);
        } else {
            // >= double → simply increase
            ref_str->cap = ref_str->len + new_data_len + 1;
            ref_str->data = realloc(ref_str->data, ref_str->cap);
        }
    }
    strcpy(ref_str->data + ref_str->len, data);
    ref_str->len += new_data_len;
    ref_str->data[ref_str->len] = '\0';
}

void str_free(str_t *ref_str) {
    free(ref_str->data);
    ref_str->cap = 0;
    ref_str->len = 0;
    ref_str->data = NULL;
}

/*
 * Go through a file searching for [ "pub" ] "fn"
 * followed by a function definition (i.e. text until "{" excluding comments)
 *
 * If we error, return 0. Compiler will catch it! Probably wasn't meant for us
 */
//
int gen_file_headers(
        const char *const fname, str_t *ref_g_hdr, str_t *ref_l_hdr) {
    FILE *file = fopen(fname, "r");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    rewind(file);
    char *code = malloc(len + 1);
    fread(code, 1, len, file);
    code[len] = '\0';
    fclose(file);

    str_t fn_header = { 0 };
    bool is_pub = false;
    for (size_t i = 0; i < len; i++) {
        if (i + 3 < len && strncmp("pub", code + i, 3) == 0 && is_wspace(code[i + 3])) {
            is_pub = true;
            i += 3;
            while (i < len && is_wspace(code[i])) {
                i++;
            }
            if (i + 2 >= len || strncmp("fn", code + i, 2) != 0) {
                // Not "pub fn", so we dont' care
                is_pub = false;
            }
        }
        if (i + 2 < len && strncmp("fn", code + i, 2) == 0 && is_wspace(code[i + 2])) {
            i += 2;
            str_new(&fn_header);

            while (i < len && code[i] != '{') {
                if (i + 1 < len && strncmp("/*", code + i, 2) == 0) {
                    // Skip to */ (or EOF)
                    while (i + 1 < len && strncmp("*/", code + i, 2) != 0) {
                        i++;
                    }
                } else if (i + 1 < len && strncmp("//", code + i, 2) == 0) {
                    // Skip to \n (or EOF)
                    while (i < len && code[i] != '\n') {
                        i++;
                    }
                } else {
                    str_push(&fn_header, code[i]);
                    i++;
                }
            }

            if (i >= len || code[i] != '{') {
                continue;
            }
            str_append(&fn_header, ";\n");
            if (is_pub) {
                str_append(ref_g_hdr, fn_header.data);
            } else {
                str_append(ref_l_hdr, fn_header.data);
            }
            str_free(&fn_header);
            is_pub = false;
        }
    }

    return 1;
}

// Saves a file. Returns 1 if no error
int save_file(const char *const fname, const char *const data) {
    FILE *file = fopen(fname, "w");
    if (!file) {
        fclose(file);
        return 0;
    }
    if (fprintf(file, "%s", data) < 0) {
        fclose(file);
        return 0;
    }
    return 1;
}

// Extract the base name of a file
char *basename(const char *const path) {
    char *str = strrchr(path, '/');
    if (!str) {
#ifdef _WIN32
        str = strrchr(path, '\\');
        if (str) {
            return strdup(str + 1);
        }
#endif
        return strdup(path);
    } else {
        return strdup(str + 1);
    }
}

bool is_wspace(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
