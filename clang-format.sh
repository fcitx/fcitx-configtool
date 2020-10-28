#!/bin/sh
find . \( -not \( -name 'locarchive.h' \) \) -a \( -name '*.h' -o -name '*.cpp' -o -name '*.c' \)  | xargs clang-format -i
