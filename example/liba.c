#include <fns.h>
#include <liba.c_fns.h>

pub fn void liba_foo(void) {
    liba_internal_bar();
}

fn void liba_internal_bar(void) {}
