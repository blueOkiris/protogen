# Protogen

## Description

Modules for C via codegen

No longer must you define headers in C. Simply call protogen on your source files and let it generate a header for you.

### A Different Way to Use C

While "Protogen C" is still C and fully compatible, it adds a couple macros that act as keywords for the protogen program to detect.

These macros evaluate to nothing and so dissapear, so they are unintrusive, but they give you the power to not worry about a whole other file for your code.

Simply include the global `fns.h` as well as your file's private `<file.c>_fns.h` where `<file.c>` is the base name of your C file, e.g. `main.c` will be like:

```c
#include <fns.h>
#include <main.c_fns.h>
#include <stdio.h>
... // And so on
```

And then you add the new keywords:

- `fn` - Functions
   + Now, when you declare functions, you can declare them in whatever order suits the code structure the best, instead of making sure you have prototypes or that certain things come before other things
   + When you go to make a function, place a `fn` keyword in front of the function definition:
      ```c
      fn int foo() {
          return 10;
      }
      ```
      It's that easy! Protogen can then use that to make a prototype and build your code.
- `deftype` - Complex Data Types
   + Data types are declared no more with `typedef` but a macro: `deftype`
   + Syntax: `deftype(name, body)`
      ```c
      deftype(
          my_struct_t,
          struct {
              int a;
              char c;
          }
      )
      ```
      Protogen will make it work under the hood and generate a typedef
- `deftype` with Generics
   + The deftype macro supports optional template generics: `deftype(name<T1, ...>, body)`
      ```c
      deftype(
          my_struct_t<T1, T2>,
          struct {
              T1 a;
              T2 b;
          }
      )
      ```
   + You then declare the type via: `<type>(T1, ...)`, e.g. `my_struct_t(int, char)` yields `struct { int a; char b; }`
- `export` - Public/Private
   + To allow other files to see your functions, types, and *also* global variables, you can "export" them.
   + This causes their info to end up in the global `fns.h` instead of your C file's local `file.c_fns.h` file.
   + E.g.
      ```c
      export fn void foo(void) {
         ...
      }
      export deftype(integer, int32_t)
      export int MY_GLOBAL = 10;
      ```

## Build

`protogen` is a single C source file, so it can be built alongside your project.

Build it first, then use it when building the rest of your code.

## Example

Check out the example (built via a well-documented script, `build_example`) to see some usages
