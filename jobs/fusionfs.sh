#!/bin/bash

function convert()
{
        myip=$1
        high=`echo $myip | cut -d '.' -f2`
        medium=`echo $myip | cut -d '.' -f3`
        low=`echo $myip | cut -d '.' -f4`
        echo $(( 100 * $high + 10 * $medium + $low ))               
}

#log this job
date >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
/home/dzhao/torusIP.sh >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt
echo "" >> /intrepid-fs0/users/dzhao/persistent/worknode_addr.txt

#generate the neighbor file for ZHT
echo "`/home/dzhao/torusIP.sh` 50000" >> /intrepid-fs0/users/dzhao/persistent/neighbor

#load libraries 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dzhao/bin/fuse/lib:/home/dzhao/bin/gbuf/lib:/home/dzhao/bin/gbuf-c/lib:/home/dzhao/fusionFS/src/udt/src:/home/dzhao/fusionFS/src/ffsnet

#start services
/home/dzhao/fusionFS/src/ffsnet/ffsnetd 2>&1 1>/dev/null &
/home/dzhao/fusionFS/src/zht/bin/server_zht 50000 /intrepid-fs0/users/dzhao/persistent/neighbor /intrepid-fs0/users/dzhao/persistent/zht.cfg TCP 2>&1 1>/dev/null &

#start fusionFS
mkdir /dev/shm/rootdir
mkdir /dev/shm/mountdir

/home/dzhao/fusionFS/src/fusionfs -o allow_other -o direct_io /dev/shm/rootdir /dev/shm/mountdir 

#####################
### run benchmarks ##
#####################
myip=`/home/dzhao/torusIP.sh`
cd /dev/shm/mountdir
RANDOM=`convert $myip`
rand=$RANDOM
lag=$(( $rand % 10 ))
sleep $lag

#mkdir 'd_'$myip
#cd 'd_'$myip

#create 1k files on 10 dirs
#start=`date +%s`
for j in {1..1} #4 processes per node
do
	dirname='d_'$myip'_'$j$
	mkdir $dirname
	./metadata_test.sh $dirname &
	#	for i in {1..5000}  
	#	do 
		#		touch 'd_'$myip'/'f_$i
		#		rm 'd_'$myip'/'f_$i
	#	done	
done
#end=`date +%s`

#diff=$(( $end - $start ))
#echo "$myip $rand $lag $start $end $diff" >> /intrepid-fs0/users/dzhao/persistent/result



#give me one hour to do something on FusionFS
sleep 3600




