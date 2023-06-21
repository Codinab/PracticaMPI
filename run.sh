#!/bin/bash

bash build.sh

ulimit -s 10485760
#ulimit -v 20000000


mpirun -np 32 ./conway -e 10 -w 10000 -h 10000
#-i Ejemplos_LifeGame/100x100/LifeGame_100x100_iter0.txt


#mpirun -np 2 ./conway -e 13 -w 10 -h 10 -i Ejemplos_LifeGame/test.txt
#mpirun -np 1 ./conway -e 10 -w 100 -h 10