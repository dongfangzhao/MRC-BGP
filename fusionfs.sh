#!/bin/bash

#get the ip address
date >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
/home/dzhao/torusIP.sh >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
echo "" >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt


#load libraries 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dzhao/bin/fuse/lib:/home/dzhao/bin/gbuf/lib:/home/dzhao/bin/gbuf-c/lib:/home/dzhao/fusionFS/src/udt/src:/home/dzhao/fusionFS/src/ffsnet

#start services
/home/dzhao/fusionFS/src/ffsnet/ffsnetd 2>&1 1>/dev/null &
/home/dzhao/fusionFS/src/zht/bin/server_zht 50000 /intrepid-fs0/users/dzhao/persistent/neighbor /intrepid-fs0/users/dzhao/persistent/zht.cfg TCP 2>&1 1>/dev/null &

#start fusionFS
mkdir -p /dev/shm/rootdir
mkdir -p /dev/shm/mountdir

/home/dzhao/fusionFS/src/fusionfs /dev/shm/rootdir /dev/shm/mountdir

#give me one hour to do something on FusionFS
sleep 3600
