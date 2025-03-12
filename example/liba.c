#include <fns.h>
#include <liba.c_fns.h>

#include <stdio.h>

export int LIB_MY_GLOBAL = 10;
int LIB_MY_LOCAL = 10;

deftype(
    a,
    struct {
        int a;
        int b;
    }
)
deftype(
    B<T1; T2>,
    struct {
        T1 a;
        T2 b;
    }
)

export fn void liba_foo(void) {
    struct { int a; char b; } c = { 0 };
    printf("%d\n", c.a);
    B(int, char) b = { 0 };
    printf("%d\n", b.a);
    liba_internal_bar();
}

fn void liba_internal_bar(void) {}
