#!/bin/bash

function convert()
{
        myip=$1
        #high=`echo $myip | cut -d '.' -f2`
        #medium=`echo $myip | cut -d '.' -f3`
        low=`echo $myip | cut -d '.' -f4`
        #echo $(( 100 * $high + 10 * $medium + $low ))
        echo $low               
}

function convert2()
{
        myip=$1
        #high=`echo $myip | cut -d '.' -f2`
        medium=`echo $myip | cut -d '.' -f3`
        #low=`echo $myip | cut -d '.' -f4`
        #echo $(( 100 * $high + 10 * $medium + $low ))
        echo $medium               
}

#get the ip address
#date >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
#/home/dzhao/torusIP.sh >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
#echo "" >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt

#generate the neighbor file for ZHT
echo "`/home/dzhao/torusIP.sh` 50000" >> /intrepid-fs0/users/dzhao/persistent/neighbor


#load libraries 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dzhao/bin/fuse/lib:/home/dzhao/bin/gbuf/lib:/home/dzhao/bin/gbuf-c/lib:/home/dzhao/fusionFS-github/src/udt/src:/home/dzhao/fusionFS-github/src/ffsnet
sleep 5
#start services
/home/dzhao/fusionFS-github/src/ffsnet/ffsnetd 2>&1 1>/dev/null &
/home/dzhao/fusionFS-github/src/zht/bin/server_zht 50000 /intrepid-fs0/users/dzhao/persistent/neighbor /intrepid-fs0/users/dzhao/persistent/zht.cfg TCP 2>&1 1>/dev/null &
#/home/dzhao/fusionFS-github/src/ffsnet/ffsnetd &
#/home/dzhao/fusionFS-github/src/zht/bin/server_zht 50000 /intrepid-fs0/users/dzhao/persistent/neighbor /intrepid-fs0/users/dzhao/persistent/zht.cfg TCP &

#start fusionFS
mkdir -p /dev/shm/rootdir
mkdir -p /dev/shm/mountdir

/home/dzhao/fusionFS-github/src/fusionfs -o allow_other -o direct_io -o big_writes -o max_write=131072 -o max_read=131072 /dev/shm/rootdir /dev/shm/mountdir 

myip=`/home/dzhao/torusIP.sh`
#RANDOM=`convert $myip`
#rand=$RANDOM
#seed=`convert $myip`
#lag=$(( $seed % 16 ))
d1=d`convert $myip`
d2=d`convert2 $myip`

if [ ! -d "/dev/shm/mountdir/$d1/$d2" ]; then
    mkdir -p /dev/shm/mountdir/$d1/$d2
fi


##########################
### metadata benchmarks ##
##########################
#start=`date +%s`
#for i in {1..10000}  
#do
#	touch $myip'_'$i 2>/dev/null
#done	
#end=`date +%s`

##########################
#### I/O Throughput ######
########################## 
startw=`/home/dzhao/fusionFS-github/bgp_script/date`
for i in {1..1}
do
	dd if=/dev/zero of=/dev/shm/mountdir/$d1/$d2/$myip'N'$i bs=1024k count=128
done
endw=`/home/dzhao/fusionFS-github/bgp_script/date`
diffw=$(echo $startw $endw | awk '{printf("%.2f", $2-$1)}')

#sleep 5 #don't read right away

#startr=`/home/dzhao/fusionFS-github/bgp_script/date`
#for i in {1..1} 
#do
#	dd of=/dev/null if=/dev/shm/mountdir/$d1/$d2/$myip'N'$i bs=1024k count=256
#done
#endr=`/home/dzhao/fusionFS-github/bgp_script/date`
#diffr=$(echo $startr $endr | awk '{printf("%.2f", $2-$1)}')

echo "$myip WRITE $diffw" >> /intrepid-fs0/users/dzhao/persistent/result_fusionfs


#give me one hour to do something on FusionFS
sleep 3600
