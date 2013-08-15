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
cd /intrepid-fs1/users/dzhao/scratch/pvfs_tmp

mkdir d_$myip
cd d_$myip

start=`date +%s`
for i in {1..1000}
do
	touch $myip'_f_'$i
#	rm $myip'_f_'$i
done
end=`date +%s`

diff=$(( $end - $start ))
echo "$myip $start $end $diff" >> /intrepid-fs1/users/dzhao/scratch/result

#give me one hour to do something on FusionFS
#sleep 3600
