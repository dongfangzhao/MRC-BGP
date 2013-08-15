#!/bin/bash

#get the ip address


date >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
/home/dzhao/torusIP.sh >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
echo "" >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt

mkdir /dev/shm/rootdir
mkdir /dev/shm/mountdir

/home/dzhao/fuse-tutorial/src/bbfs /dev/shm/rootdir /dev/shm/mountdir

#give me one hour to do something on FusionFS
sleep 3600
