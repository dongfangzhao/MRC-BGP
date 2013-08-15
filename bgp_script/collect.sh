#!/bin/bash
rm all.data
FILES=/home/dzhao/persistent/rootdir/*

for f in ${FILES}
do
	cat ${f}/log >> all.data
done
