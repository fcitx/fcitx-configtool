#!/bin/sh
find . \( -not \( -name 'reserved.h' -o -name 'reserved.c' \) \) -a \( -name '*.h' -o -name '*.cpp' -o -name '*.c' \)  | xargs clang-format -i
