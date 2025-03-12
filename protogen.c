// Protogen prototype generator

#define VERS        1L

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
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("%lu\n", VERS);
        return 0;
    }
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
        "#pragma once\n\n#define fn \n#define export \n#define deftype(a, b)\n"
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
        if (ref_str->len + new_data_len + 1 < (ref_str->len + 1) * 2) {
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
 * Go through a file searching for [ "export" ] "fn"
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
        if (i + 6 < len && strncmp("export", code + i, 6) == 0 && is_wspace(code[i + 6])) {
            is_pub = true;
            i += 6;
            while (i < len && is_wspace(code[i])) {
                i++;
            }
            if ((i + 2 >= len || strncmp("fn", code + i, 2) != 0)
                    && (i + 7 >= len || strncmp("deftype", code + i, 7) != 0)) {
                // Not "export fn" or "export deftype,"
                // so must be an exported global var
                bool found_eq = false;
                str_new(&fn_header);
                str_append(&fn_header, "extern ");
                while (i < len && code[i] != ';') {
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
                        if (code[i] == '=') {
                            found_eq = true;
                        }
                        if (!found_eq) {
                            str_push(&fn_header, code[i]);
                        }
                        i++;
                    }
                }
                if (i >= len || code[i] != ';') {
                    str_free(&fn_header);
                    is_pub = false;
                    continue;
                }
                str_append(&fn_header, ";\n");
                str_append(ref_g_hdr, fn_header.data);
                str_free(&fn_header);
                is_pub = false;
                continue;
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
                str_free(&fn_header);
                is_pub = false;
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
        } else if (i + 7 < len && strncmp("deftype", code + i, 2) == 0
                && (is_wspace(code[i + 7]) || code[i + 7] == '(')) {
            while (i < len && code[i] != '(') {
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
                    i++;
                }
            }
            if (i >= len || code[i] != '(') {
                str_free(&fn_header);
                is_pub = false;
                continue;
            }
            i++;

            str_t name = { 0 };
            str_new(&name);
            while (i < len && code[i] != ',' && code[i] != '<') {
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
                    if (!is_wspace(code[i])) {
                        str_push(&name, code[i]);
                    }
                    i++;
                }
            }
            if (i >= len || (code[i] != ',' && code[i] != '<')) {
                str_free(&name);
                is_pub = false;
                continue;
            }
            i++;

            bool has_gens = false;
            str_t gens;
            str_new(&gens);
            if (code[i - 1] == '<') {
                has_gens = true;
                while (i < len && code[i] !=  '>') {
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
                        if (code[i] != ';' && !is_wspace(code[i])) {
                            str_push(&gens, code[i]);
                        } else if (code[i] == ';') {
                            str_push(&gens, ',');
                        }
                        i++;
                    }
                }
                if (i >= len || code[i] != '>') {
                    str_free(&name);
                    str_free(&gens);
                    is_pub = false;
                    continue;
                }
                i++;

                while (i < len && code[i] !=  ',') {
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
                        i++;
                    }
                }
                if (i >= len || code[i] != ',') {
                    str_free(&name);
                    str_free(&gens);
                    is_pub = false;
                    continue;
                }
                i++;
            }

            str_t body = { 0 };
            str_new(&body);
            while (i < len && code[i] != ')') {
                if (i + 1 < len && strncmp("/*", code + i, 2) == 0) {
                    // Skip to */ (or EOF)
                    while (i + 1 < len && strncmp("*/", code + i, 2) != 0) {
                        if (has_gens && code[i] == '\n') {
                            str_append(&body, " \\\n");
                        }
                        i++;
                    }
                } else if (i + 1 < len && strncmp("//", code + i, 2) == 0) {
                    // Skip to \n (or EOF)
                    while (i < len && code[i] != '\n') {
                        i++;
                    }
                } else {
                    if (has_gens && code[i] == '\n') {
                        str_append(&body, " \\\n");
                    } else {
                        str_push(&body, code[i]);
                    }
                    i++;
                }
            }
            if (i >= len || code[i] != ')') {
                str_free(&name);
                str_free(&gens);
                str_free(&body);
                is_pub = false;
                continue;
            }
            i++;

            str_new(&fn_header);
            if (has_gens) {
                str_append(&fn_header, "#define __");
                str_append(&fn_header, name.data);
                str_append(&fn_header, "__unwrap_va_args(");
                str_append(&fn_header, gens.data);
                str_append(&fn_header, ") \\\n");
                str_append(&fn_header, body.data);
                str_append(&fn_header, "\n#define ");
                str_append(&fn_header, name.data);
                str_append(&fn_header, "(...) __");
                str_append(&fn_header, name.data);
                str_append(&fn_header, "__unwrap_va_args(__VA_ARGS__)\n");
            } else {
                str_append(&fn_header, "typedef\n");
                str_append(&fn_header, body.data);
                str_push(&fn_header, '\n');
                str_append(&fn_header, name.data);
                str_append(&fn_header, ";\n");
            }
            if (is_pub) {
                str_append(ref_g_hdr, fn_header.data);
            } else {
                str_append(ref_l_hdr, fn_header.data);
            }

            str_free(&fn_header);
            str_free(&name);
            str_free(&gens);
            str_free(&body);
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
