#!/bin/bash

current_dir=$(dirname "$0")
cd "$current_dir/build/" || exit

make install




