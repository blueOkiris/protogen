// A simple implementation of a vector

#include <fns.h>
#include <vec.c_fns.h>
#include <stdint.h>
#include <stddef.h>

export deftype(
    Vec<T>,
    struct {
        size_t cap;
        size_t len;
        T *data;
    }
)


void vec_new(Vec(T) ) // HOW????
