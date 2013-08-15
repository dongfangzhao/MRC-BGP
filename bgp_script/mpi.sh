#!/bin/sh

echo hi
cobalt-mpirun -mode smp -np 16 /home/dzhao/fusionFS-github/bgp_script/pi
