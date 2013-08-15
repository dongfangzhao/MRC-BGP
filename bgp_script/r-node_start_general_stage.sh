#!/bin/bash

#r-stage-node-start
#Prepare runing code
#echo "Start..."
#server="server_general_queued"
server="server_zht"
#"/home/dzhao/fusionFS-staging/src/zht/bin/server_zht"
cfg="/home/dzhao/fusionFS-staging/src/zht/zht.cfg"
#client="benchmark_client"
client="zht_benchmark_d"

numNode=$1
#TCP=$2
#prefix=$3
#k_len=$4
#v_len=$5


mem_TCP=8192 #buffer size for each TCP connection, both read and write
cp /home/dzhao/fusionFS-staging/src/zht/bin/server_zht /dev/shm/server_zht
#cp $client /dev/shm/.
cp torusIP.sh /dev/shm/torusIP
cp $cfg /dev/shm/.
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tonglin/Installed/built/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dzhao/bin/fuse/lib:/home/dzhao/bin/gbuf/lib:/home/dzhao/bin/gbuf-c/lib:/home/dzhao/fusionFS-staging/src/udt/src:/home/dzhao/fusionFS-staging/src/ffsnet
#cat /proc/sys/net/core/somaxconn
ulimit -n 65536 #The maximum number of open file descriptors.
ulimit -s unlimited #The maximum stack size.
ulimit -l unlimited #The maximum size that may be locked into memory.
ulimit -c unlimited #The maximum size of core files created.
ulimit -m unlimited #The maximum resident set size. 
#echo "Original settings:"
#cat /proc/sys/net/core/somaxconn
#cat /proc/sys/net/core/netdev_max_backlog
#cat /proc/sys/net/ipv4/tcp_fin_timeout
#cat /proc/sys/net/ipv4/ip_local_port_range
echo 16387 > /proc/sys/kernel/threads-max
echo 32780 > /proc/sys/net/core/somaxconn #limit of listen queue, default is 128
echo 32780 > /proc/sys/net/core/netdev_max_backlog
echo 1 > /proc/sys/net/ipv4/tcp_fin_timeout
echo "10000   65000" > /proc/sys/net/ipv4/ip_local_port_range

echo 40960 > /proc/sys/net/ipv4/neigh/default/gc_thresh2
echo 40960 > /proc/sys/net/ipv4/neigh/default/gc_thresh3

echo 1 > /proc/sys/net/ipv4/tcp_timestamps 	#sysctl -w net.ipv4.tcp_timestamps=1
echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle 	#sysctl -w net.ipv4.tcp_tw_recycle=1
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse	#sysctl -w net.ipv4.tcp_tw_reuse=1

######################### below for 8192 
echo $mem_TCP > /proc/sys/net/core/rmem_max
echo $mem_TCP > /proc/sys/net/core/wmem_max
echo $mem_TCP > /proc/sys/net/core/wmem_default
echo $mem_TCP > /proc/sys/net/core/rmem_default


#echo "24576   32768   49152" > /proc/sys/net/ipv4/tcp_mem
#echo "16384     18000   20000" > /proc/sys/net/ipv4/tcp_mem # 16384 = 1GB
echo "4096	8192	12288" > /proc/sys/net/ipv4/tcp_mem #12288=768MB
#echo "Memory page size = `getconf PAGE_SIZE`" page size = 65536.
#echo "/proc/sys/fs/file-max = `cat /proc/sys/fs/file-max`"
echo 201510 > /proc/sys/fs/file-max
#echo "/proc/sys/net/ipv4/tcp_mem = `cat /proc/sys/net/ipv4/tcp_mem`"
#echo "/proc/sys/net/ipv4/tcp_rmem = `cat /proc/sys/net/ipv4/tcp_rmem`"
#echo "/proc/sys/net/ipv4/tcp_wmem = `cat /proc/sys/net/ipv4/tcp_rmem`"

#echo "New settings:====================================="
#cat /proc/sys/net/core/somaxconn
#cat /proc/sys/net/core/netdev_max_backlog
#cat /proc/sys/net/ipv4/tcp_fin_timeout
#cat /proc/sys/net/ipv4/ip_local_port_range
#echo "initial mem usage: `free `"

#numNode=$1
#TCP=$2

numInstance=1
numZHT=`expr $numNode \* $numInstance`
#ip=`./ip.sh` # Within 1 partition (64 nodes)
ip=`/dev/shm/torusIP`
#port=`expr 50000 + $numNode`
port=50000
port2=50002
port3=50003
port4=50004
#echo $ip
#touch /intrepid-fs0/users/tonglin/persistent/$numNode/$ip
#echo "$ip"
export IP=$ip
export NNODE=$(($numNode *  $numInstance ))
export PORT_FOR_REPLICA=50009
jobID=$(head -1 /fuse/tmp/jobID)
#echo "jobID = $jobID"
#echo "Time: Start: `date`"
sleep 10
#echo "$ip $port" >>  /fuse/tmp/neighbors/neighbor_$numZHT
/home/dzhao/fusionFS-staging/src/ffsnet/ffsnetd 2>&1 1>/dev/null &
echo "`/home/dzhao/torusIP.sh` 50000" >>  /fuse/tmp/neighbors/neighbor_$numZHT
#echo "$ip $port2" >>  /fuse/tmp/neighbors/neighbor_$numZHT
#echo "$ip $port3" >>  /fuse/tmp/neighbors/neighbor_$numZHT
#echo "$ip $port4" >>  /fuse/tmp/neighbors/neighbor_$numZHT
sleep 20 # 3 for small, 10 for large
#echo "check neighbor list"
numLine=`wc -l /fuse/tmp/neighbors/neighbor_$numZHT | awk {'print $1'} `
# check if neighbor list is ready.
#echo "--------------1"
while [[ "$numLine" != "$numZHT" ]]
do
	sleep 1 # 2 for small, 5 for large
#	echo "Neighbotlist: --------------should be $numZHT, numLine = $numLine"
	numLine=`wc -l /fuse/tmp/neighbors/neighbor_$numZHT | awk {'print $1'} `
done
#echo "--------------3"
cp /fuse/tmp/neighbors/neighbor_$numZHT /dev/shm/neighbor
#echo "Time: neighbor list ready: `date`"

###############################
#/dev/shm/$server $port /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg  $TCP tonglin  &
/dev/shm/$server 50000 /dev/shm/neighbor /dev/shm/zht.cfg TCP 2>&1 1>/dev/null &

sleep 1
#/dev/shm/$server $port2 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg  $TCP tonglin  &
#sleep 1
#/dev/shm/$server $port3 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg  $TCP tonglin  &
#sleep 1
#/dev/shm/$server $port4 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg  $TCP tonglin  &
#sleep 1
###############################
sleep 5
#echo "server started on $ip"
nLine=0
#Check if all servers are started

while [[ "$nLine" != "$numZHT" ]]

do
        sleep 2
	nLine=`wc -l /fuse/tmp/reg | awk {'print $1'} `
	#reg_$numZHT
#	echo "Register: --------------should be $numZHT, nLine = $nLine"
done
#echo "Time: Server start complete: `date`"
sleep 10
 
#sleep $numNode
#echo "About to start client on $ip"
#/dev/shm/$client 10000 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg $TCP >> /fuse/tmp/results/$IP.$port &
#/dev/shm/$client 10000 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg $TCP >> /fuse/tmp/results/$IP.$port &
#/dev/shm/$client 10000 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg $TCP >> /fuse/tmp/results/$IP.$port &


#/dev/shm/$client 10000 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg $TCP >> /fuse/tmp/results/$IP.$port


#/dev/shm/$client 10000 /dev/shm/neighbor_$numZHT /dev/shm/zht.cfg $TCP $prefix $k_len $v_len
mkdir -p /dev/shm/rootdir
mkdir -p /dev/shm/mountdir

/home/dzhao/fusionFS-staging/src/fusionfs -o allow_other -o direct_io -o big_writes -o max_write=131072 -o max_read=131072 /dev/shm/rootdir /dev/shm/mountdir

function convertl()
{
	myip=$1
	#high=`echo $myip | cut -d '.' -f2`
	#medium=`echo $myip | cut -d '.' -f3`
	low=`echo $myip | cut -d '.' -f4`
	#echo $(( 100 * $high + 10 * $medium + $low ))
	echo $low               
}

function convertm()
{
	myip=$1
	#high=`echo $myip | cut -d '.' -f2`
	medium=`echo $myip | cut -d '.' -f3`
	#low=`echo $myip | cut -d '.' -f4`
	#echo $(( 100 * $high + 10 * $medium + $low ))
	echo $medium               
}

function converth()
{
	myip=$1
	high=`echo $myip | cut -d '.' -f2`
	#medium=`echo $myip | cut -d '.' -f3`
	#low=`echo $myip | cut -d '.' -f4`
	#echo $(( 100 * $high + 10 * $medium + $low ))
	echo $high               
}

dl=d`convertl $ip`
dm=d`convertm $ip`
dh=d`converth $ip`
mkdir -p /dev/shm/mountdir/$dh/$dm/$dl

startw=`date +%s`
RES="$ip $startw"

#DFZ: I/O throughput benchmark, 64GB per node
for i in {1..64}
do
	dd if=/dev/zero of=/dev/shm/mountdir/$dh/$dm/$dl/f_$ip bs=1024k count=256
	endw=`date +%s`
	RES="$RES $endw"
done

#DFZ: metadata benchmark
#for i in {1..100}
#do
#	touch /dev/shm/mountdir/d_${ip}/f_${i}
#done
#endw=`date +%s`
#RES="$RES $endw"

#DFZ: save the result to I/O nodes and flush it to GPFS when the job is completed
echo $RES >> /fuse/tmp/results/data


#DFZ: if we want to save the result directly to GPFS:
#echo $RES >> /home/dzhao/persistent/result_fusionfs_$jobID

#startw=`/home/dzhao/fusionFS-staging/bgp_script/date`
#for i in {1..10}
#do
#     dd if=/dev/zero of=/dev/shm/mountdir/$ip bs=1024k count=128
#done
#endw=`/home/dzhao/fusionFS-staging/bgp_script/date`
#diffw=$(echo $startw $endw | awk '{printf("%.2f", $2-$1)}')

#echo "$myip WRITE $startw $endw $diffw" >> /intrepid-fs0/users/dzhao/persistent/result_fusionfs-$jobID  # /fuse/tmp/results/data 
#echo "$myip WRITE $startw $endw $diffw" >> /intrepid-fs0/users/dzhao/persistent/result_fusionfs


sleep 3600  
#echo "client etarted on $ip"
