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
MYIP=`/home/dzhao/torusIP.sh`

echo -n "${MYIP}," >> /intrepid-fs0/users/dzhao/persistent/neighbor
sleep 5

#DFZ: setup PVFS server
SERVERS=`cat /home/dzhao/persistent/neighbor`
/home/dzhao/bin/pvfs2/bin/pvfs2-genconfig --protocol tcp --ioservers ${SERVERS} --metaservers ${SERVERS} --quiet /home/dzhao/persistent/pvfs2-fs.conf 
sleep 30 
/home/dzhao/bin/pvfs2/sbin/pvfs2-server /home/dzhao/persistent/pvfs2-fs.conf -f -a ${MYIP}
/home/dzhao/bin/pvfs2/sbin/pvfs2-server /home/dzhao/persistent/pvfs2-fs.conf -a ${MYIP}

#DFZ: setup PVFS client
mkdir -p /mnt/pvfs2
touch /etc/pvfs2tab
chmod a+r /etc/pvfs2tab
echo "tcp://${MYIP}:3334/pvfs2-fs /mnt/pvfs2 pvfs2 defaults,noauto 0 0" > /etc/pvfs2tab

#DFZ: setup FUSE
mkdir /pvfs
/home/dzhao/bin/pvfs2/bin/pvfs2fuse /pvfs 

cp /home/dzhao/fusionFS-github/bgp_script/date ./

#DFZ: I/O benchmark
#STARTW=`./date`
#	dd if=/dev/zero of=/pvfs/f_${MYIP} bs=1024k count=128
#ENDW=`./date`
#DIFFW=$(echo $STARTW $ENDW | awk '{printf("%.2f", $2-$1)}')

#STARTR=`./date`
#	dd of=/dev/null if=/pvfs/f_${MYIP} bs=1024k count=128
#ENDR=`./date`
#DIFFR=$(echo $STARTR $ENDR | awk '{printf("%.2f", $2-$1)}')

#echo "PVFS-WRITE IP-${MYIP} ${STARTW} ${ENDW} ${DIFFW}" >> /home/dzhao/persistent/result_pvfs_w
#echo "PVFS-READ IP-${MYIP} ${STARTR} ${ENDR} ${DIFFR}" >> /home/dzhao/persistent/result_pvfs_r

#DFZ: metadata benchmark (single directory)
#START=`./date`
#for i in {1..10000}
#do
#	touch /pvfs/${MYIP}_${i} 2>/dev/null
#done
#END=`./date`
#DIFF=$(echo $START $END | awk '{printf("%.2f", $2-$1)}')
#echo "PVFS-META-SINGLE IP-${MYIP} ${START} ${END} ${DIFF}" >> /home/dzhao/persistent/result_pvfs_$1

#DFZ: metadata benchmark (many directory)
mkdir -p /pvfs/dir_${MYIP}
START=`./date`
for i in {1..10000}
do
	touch /pvfs/dir_${MYIP}/${MYIP}_${i} 2>/dev/null
done
END=`./date`
DIFF=$(echo $START $END | awk '{printf("%.2f", $2-$1)}')
echo "PVFS-META-MANY IP-${MYIP} ${START} ${END} ${DIFF}" >> /home/dzhao/persistent/result_pvfs_$1
#load libraries 
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dzhao/bin/fuse/lib:/home/dzhao/bin/gbuf/lib:/home/dzhao/bin/gbuf-c/lib:/home/dzhao/fusionFS-github/src/udt/src:/home/dzhao/fusionFS-github/src/ffsnet

#sleep 5
#start services
#/home/dzhao/fusionFS-github/src/ffsnet/ffsnetd 2>&1 1>/dev/null &
#/home/dzhao/fusionFS-github/src/zht/bin/server_zht 50000 /intrepid-fs0/users/dzhao/persistent/neighbor /intrepid-fs0/users/dzhao/persistent/zht.cfg TCP 2>&1 1>/dev/null &
#/home/dzhao/fusionFS-github/src/ffsnet/ffsnetd &
#/home/dzhao/fusionFS-github/src/zht/bin/server_zht 50000 /intrepid-fs0/users/dzhao/persistent/neighbor /intrepid-fs0/users/dzhao/persistent/zht.cfg TCP &

#start fusionFS
#mkdir -p /dev/shm/rootdir
#mkdir -p /dev/shm/mountdir

#/home/dzhao/fusionFS-github/src/fusionfs -o allow_other -o direct_io /dev/shm/rootdir /dev/shm/mountdir 


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
#startw=`/home/dzhao/fusionFS-github/bgp_script/date`
#for i in {1..1}
#do
#	dd if=/dev/zero of=/dev/shm/mountdir/$d1/$d2/$myip'N'$i bs=1024k count=128
#done
#endw=`/home/dzhao/fusionFS-github/bgp_script/date`
#diffw=$(echo $startw $endw | awk '{printf("%.2f", $2-$1)}')

#sleep 5 #don't read right away

#startr=`/home/dzhao/fusionFS-github/bgp_script/date`
#for i in {1..1} 
#do
#	dd of=/dev/null if=/dev/shm/mountdir/$d1/$d2/$myip'N'$i bs=1024k count=256
#done
#endr=`/home/dzhao/fusionFS-github/bgp_script/date`
#diffr=$(echo $startr $endr | awk '{printf("%.2f", $2-$1)}')

#echo "$myip WRITE $diffw READ $diffr" >> /intrepid-fs0/users/dzhao/persistent/result_fusionfs


#give me one hour to do something on FusionFS
sleep 3600
