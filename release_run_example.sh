#!/bin/sh

make || exit 1

echo "Running vismut (release):"
./build/release/vismut compile -i ./example/main.bi -o ./example/main.bic

