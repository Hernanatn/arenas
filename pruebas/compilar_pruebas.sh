#!/bin/bash

correr=false

while getopts "c" opt; do
    case $opt in
        c)
            correr=true
            ;;
        *)
            ;;
    esac
done

CXX_FLAGS="-I../externos/Catch2/src -I../fuente -std=c++20"
BUILD_DIR="build/wsl" 

cmake -S . -B $BUILD_DIR -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="$CXX_FLAGS"
cmake --build $BUILD_DIR

rm -f "pruebas/pruebas"
mv "$BUILD_DIR/correr_pruebas" "pruebas/pruebas"

if [ "$correr" = true ]; then
    valgrind --leak-check=full --show-leak-kinds=all ./pruebas/pruebas -s
fi
