#!/bin/sh
# Build the example project
# Could be a Makefile, but I wanted the build steps to be more clear for all
# Requires gcc

mkdir -p out

# First, build protogen
echo "Compiling protogen..."
gcc -Wall -Werror -g -o out/protogen protogen.c

# Run protogen on all the src w/ out/ as output dir to generate headers
echo "Generating headers..."
./out/protogen out example/*.c

# Build each source file using generated headers
echo "Compiling C source..."
for f in example/*.c; do
    gcc -Wall -Werror -g -c "$f" -o "out/$(basename "$f" .c).o" -Iout
done

# Link
echo "Linking"
gcc -o out/example out/*.o

echo "Done."
