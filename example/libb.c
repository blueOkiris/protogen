#include <fns.h>
#include <libb.c_fns.h>

pub fn void libb_foo(void) {
    libb_internal_bar();
}

fn void libb_internal_bar(void) {}
