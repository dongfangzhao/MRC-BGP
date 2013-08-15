#!/bin/bash
function convert()
{
        myip=$1
        high=`echo $myip | cut -d '.' -f2`
        medium=`echo $myip | cut -d '.' -f3`
        low=`echo $myip | cut -d '.' -f4`
        echo $(( 100 * $high + 10 * $medium + $low ))               
}

myip=`/home/dzhao/torusIP.sh`

for j in {1..4} #1 processes per node 
do
	/home/dzhao/fusionFS-github/jobs/gpfs_meta_thread.sh $myip $j &
done

#give me one hour to do something on FusionFS
sleep 3600
