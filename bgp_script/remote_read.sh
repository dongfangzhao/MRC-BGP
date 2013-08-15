#!/bin/bash

for i in {1..10}; do dd of=/dev/null if="/dev/shm/mountdir/f16m$i" bs=1024k count=16; done
