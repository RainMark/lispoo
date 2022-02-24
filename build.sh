#!/bin/bash

# git-clang-format --style=Google HEAD^

g++ -Werror -std=c++20 -Isrc src/lispoo.cpp -o lispoo
