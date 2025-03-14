#include <fns.h>
#include <main.c_fns.h>
#include <stddef.h>
#include <stdio.h>

export fn int main(int argc, char **argv) {
    printf("Creating vec of capacity 2...\n");
    Vec(int) vec = { 0 };
    vec_new(REFVEC(vec), sizeof(int), 2);
    printf(
#ifdef _WIN32
        "Vec { cap: %llu, len: %llu, [] }\n",
#else
        "Vec { cap: %lu, len: %lu, [] }\n",
#endif
        vec.cap, vec.len
    );

    printf("Filling it to [ 1, 2, 3, 4 ]...\n");
    int elem = 1;
    vec_push(&vec.cap, &vec.len, (void **) &vec.data, sizeof(int), &elem);
    printf(
#ifdef _WIN32
        "Vec { cap: %llu, len: %llu, [ %d ] }\n",
#else
        "Vec { cap: %lu, len: %lu, [ %d ] }\n",
#endif
        vec.cap, vec.len, vec.data[0]
    );
    elem = 2;
    vec_push(&vec.cap, &vec.len, (void **) &vec.data, sizeof(int), &elem);
    printf(
#ifdef _WIN32
        "Vec { cap: %llu, len: %llu, [ %d, %d ] }\n",
#else
        "Vec { cap: %lu, len: %lu, [ %d ] }\n",
#endif
        vec.cap, vec.len, vec.data[0], vec.data[1]
    );
    elem = 3;
    vec_push(&vec.cap, &vec.len, (void **) &vec.data, sizeof(int), &elem);
    printf(
#ifdef _WIN32
        "Vec { cap: %llu, len: %llu, [ %d, %d, %d ] }\n",
#else
        "Vec { cap: %lu, len: %lu, [ %d ] }\n",
#endif
        vec.cap, vec.len, vec.data[0], vec.data[1], vec.data[2]
    );
    elem = 4;
    vec_push(&vec.cap, &vec.len, (void **) &vec.data, sizeof(int), &elem);
    printf(
#ifdef _WIN32
        "Vec { cap: %llu, len: %llu, [ %d, %d, %d, %d ] }\n",
#else
        "Vec { cap: %lu, len: %lu, [ %d, %d, %d, %d ] }\n",
#endif
        vec.cap, vec.len, vec.data[0], vec.data[1], vec.data[2], vec.data[3]
    );

    printf("Popping...\n");
    vec_pop(&vec.cap, &vec.len, (void **) &vec.data);
    printf(
#ifdef _WIN32
        "Vec { cap: %llu, len: %llu, [ %d, %d, %d ] }\n",
#else
        "Vec { cap: %lu, len: %lu, [ %d, %d, %d ] }\n",
#endif
        vec.cap, vec.len, vec.data[0], vec.data[1], vec.data[2]
    );

    printf("Freeing...\n");
    vec_free(&vec.cap, &vec.len, (void **) vec.data);
    return 0;
}
