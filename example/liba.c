#include <fns.h>
#include <liba.c_fns.h>

export int LIB_MY_GLOBAL = 10;
int LIB_MY_LOCAL = 10;

export fn void liba_foo(void) {
    liba_internal_bar();
}

fn void liba_internal_bar(void) {}
