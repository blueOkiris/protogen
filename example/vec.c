// A simple implementation of a vector

#include <fns.h>
#include <vec.c_fns.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

export deftype(
    Vec<T>,
    struct {
        size_t cap;
        size_t len;
        T *data;
    }
)

export def(REFVEC(vec) &vec.cap, &vec.len, (void **) &vec.data)

export fn void vec_new(
        size_t *ref_cap, size_t *ref_len, void **ref_data, size_t data_size,
        size_t capacity) {
    *ref_cap = capacity;
    *ref_len = 0;
    *ref_data = malloc(data_size * capacity);
}

export fn void vec_push(
        size_t *ref_cap, size_t *ref_len, void **ref_data, size_t data_size,
        void *elem) {
    if (*ref_len + 1 > *ref_cap) {
        *ref_cap *= 2;
        printf("New cap: %llu\n", *ref_cap);
        *ref_data = realloc(*ref_data, data_size * (*ref_cap));
    }
    memcpy(*ref_data + data_size * (*ref_len), elem, data_size);
    (*ref_len)++;
}

export fn void vec_pop(size_t *ref_cap, size_t *ref_len, void **ref_data) {
    (*ref_len)--;
}

export fn void vec_free(size_t *ref_cap, size_t *ref_len, void **ref_data) {
    *ref_cap = 0;
    *ref_len = 0;
    free(*ref_data);
    *ref_data = NULL;
}
