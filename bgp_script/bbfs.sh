#!/bin/bash



echo `date`
echo ""
MYIP=`/home/dzhao/torusIP.sh`
echo "Compute node IP: "${MYIP}
echo ""

#give me the resource
ulimit -n 65536 		#The maximum number of open file descriptors.
ulimit -s unlimited 	#The maximum stack size.
ulimit -l unlimited 	#The maximum size that may be locked into memory.
ulimit -c unlimited 	#The maximum size of core files created.
ulimit -m unlimited 	#The maximum resident set size. 

#mkdir /dev/shm/rootdir
#rm -rf /home/dzhao/persistent/rootdir/${MYIP}
mkdir -p /home/dzhao/persistent/rootdir/${MYIP}
mkdir /dev/shm/mountdir

/home/dzhao/pnnl2013/bbfs -o big_writes /home/dzhao/persistent/rootdir/${MYIP} /dev/shm/mountdir

#run tests on the file system
cp /home/dzhao/persistent/temperature.data /dev/shm/temperature.data
cp /home/dzhao/torusIP.sh /dev/shm/torusIP.sh
/home/dzhao/pnnl2013/test_simple_io ${MYIP}

#give me one hour to do something on FusionFS
sleep 3600
