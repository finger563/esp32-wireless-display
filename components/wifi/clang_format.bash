#!/usr/bin/env bash
find ./src ./include -type f \( -iname \*.cpp -o -iname \*.hpp \) | xargs clang-format -style="{ColumnLimit : 100}" -i
