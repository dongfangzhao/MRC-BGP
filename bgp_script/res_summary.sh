#!/bin/bash

NODES=$1
JOBID=$2

if [[ -z $NODES || -z $JOBID ]] 
then
	echo "Usage: $0 <NODES> <JOBID>"
	exit
fi

RES="${JOBID}.summary"
rm $RES
touch $RES
BATCH=$(( NODES / 64 ))
for (( i=0; i<$BATCH; i++))
do
	echo "Processing IO-Node #$i"
	cat ~/persistent/staging/IO-${i}/results/${JOBID} >> $RES
done
