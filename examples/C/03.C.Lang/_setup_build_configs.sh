#!/bin/bash

rm -rf ./build

meson setup --buildtype debug                    ./build/debug   .
meson setup --buildtype release --optimization 3 ./build/release .
