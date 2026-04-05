#!/bin/sh

make || exit 1

echo "Running vismut (release):"
./build/release/vismut ./example/main.vismut

