# Protogen

## Description

Modules for C via codegen

No longer must you define headers for function prototypes in C. Simply call protogen on your source files and let it generate a header for you.

### How It Works

You declare functions in C via the new `fn` and `pub` keywords, i.e. `pub fn int main(int argc, char **argv) { ... }`.

Then, protogen uses that to create a couple headers:

- A single, global header `fns.h` with all public functions (`pub fn`)
- A file for each c file called `<c file name>_fns.h` for local prototypes (just `fn`)

### Example

Consider a library set up something like this with basic organization:

- liba.h
   ```c
   #pragma once

   // Public API
   void liba_foo(void);
   ```
- liba.c
   ```c
   #include <liba.h>

   // Private protos
   void internal_bar(void);
   ...

   // Public API
   void liba_foo(void) {
       ...
       internal_bar();
       ...
   }
   ...

   // Private fns
   void internal_bar(void) {
       ...
   }
   ```
- liba.h
   ```c
   #pragma once

   // Public API
   void libb_foo(void);
   ```
- libb.c
   ```c
   #include <libb.h>

   // Private protos
   void internal_bar(void);
   ...

   // Public API
   void libb_foo(void) {
       ...
       internal_bar();
       ...
   }
   ...

   // Private fns
   void internal_bar(void) {
       ...
   }
   ```
- main.c:
   ```
   #include <liba.h>
   #include <libb.h>

   int main(char **argv, char **argc) {
      liba_foo();
      libb_foo();
   }
   ```

And it just gets more complex from there: extra files, reduplicated naming and code, etc

You could use macros to try to do it, but then you forgo the help of tools for error checking and such. Debuggin macros is not the way to go.

Try the protogen way instead. Your project will look like this instead:

- liba.c
   ```c
   #include <fns.h>
   #include <liba_fns.h>

   // Public API
   pub fn void liba_foo(void) {
       ...
       internal_bar();
       ...
   }
   ...

   // Private fns
   fn void internal_bar(void) {
       ...
   }
   ```
- libb.c
   ```c
   #include <fns.h>
   #include <libb_fns.h>

   // Public API
   pub fn void libb_foo(void) {
       ...
       internal_bar();
       ...
   }
   ...

   // Private fns
   fn oid internal_bar(void) {
       ...
   }
   ```
- main.c:
   ```
   #include <fns.h>
   #include <main_fns.h>

   pub fn int main(char **argv, char **argc) {
      liba_foo();
      libb_foo();
   }
   ```

It's just a lot cleaner, and the only cost is you have to run `protogen out/ src/liba.c src/libb.c src/main.c` before calling gcc/msvc/clang in your build process.

It will generate `out/fns.h`, `out/main_fns.h`, `out/liba_fns.h`, and `out/libb_fns.h`.

## Build

`protogen` is a single C source file, so it can be built alongside your project.

Build it first, then use it when building the rest of your code.
