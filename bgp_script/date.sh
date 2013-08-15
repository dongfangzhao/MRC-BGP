#! /bin/bash

sec=`date +%s`
nsec=`date +%N`
msec=$(( $nsec / 1000000 ))
echo $sec'.'$msec

