#!/bin/bash

DIRNAME=$1

start=`date +%s`
for i in {1..5000}  
do 
	echo doing $i
	touch $DIRNAME'/'f_$i
	rm $DIRNAME'/'f_$i
done
end=`date +%s`

diff=$(( $end - $start ))
echo "$DIRNAME $start $end $diff" >> /intrepid-fs0/users/dzhao/persistent/result