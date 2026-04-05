#!/bin/bash

make debug || exit 1

echo "Starting LLDB session for Vismut..."

lldb -o "b ASTParser_ParseType" -o "run ./example/main.vismut" -- ./build/debug/vismut-debug
