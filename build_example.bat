@echo off
REM Build the example project
REM Could be a Makefile, but I wanted the build steps to be more clear for all
REM Requires mingw32/mingw64 installed w/ gcc in PATH

if not exist out mkdir out

REM First, build protogen
echo Compiling protogen...
gcc -Wall -Werror -g -o out\protogen.exe protogen.c

REM Run protogen on all the src w/ out/ as output dir to generate headers
echo Generating headers...
out\protogen.exe out example\liba.c example\libb.c example\main.c

REM  Build each source file using generated headers
echo Compiling C source...
for %%f in (example\*.c) do gcc -Wall -Werror -g -c "%%f" -o "out\%%~nf.o" -Iout

REM Link
echo Linking
gcc -o out\example.exe out\main.o out\liba.o out\libb.o
