#!/bin/bash

make debug || exit 1

echo "Starting LLDB session for Vismut..."

lldb -o "b VismutCompiler_Compile" -o "run compile -i ./example/main.bi -o ./example/main.bic" -- ./build/debug/vismut-debug
