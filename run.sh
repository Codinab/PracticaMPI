#!/bin/bash

bash build.sh

ulimit -s 20000

mpirun -np 1 ./conway
