#!/bin/sh
#ZOID start script
para=$1 #1 for start, 0 for shutdown

logPath="/intrepid-fs0/users/dzhao/persistent/IOtest" 
base="/intrepid-fs0/users/dzhao/persistent/staging"
rank=$(grep BG_PSETNUM /proc/personality.sh | cut -d '=' -f 2)
jobID=$(echo $ZOID_JOB_ENV | sed 's/^.*COBALT_JOBID=\([^:]*\).*$/\1/')
#rm /intrepid-fs0/users/tonglin/persistent/IO_test
	ifconfig >> $base/IO-$rank/info.$jobID
if [ $para -eq 0 ]; then
#        rm -fr /tmp/neighbors/*
#        rm -fr /tmp/registers/*
        
        #echo "" >> /intrepid-fs0/users/tonglin/persistent/IO_test
#       echo "para=0, IO node shutdown..." >> $logPath
	cat /tmp/results/* >> /intrepid-fs0/users/dzhao/persistent/staging/IO-$rank/results/$jobID
	sleep 5

fi

if [ $para -eq 1 ]; then
    # 1
        #1: IO node start:
#	echo "para=1, IO node start..." >> $logPath
#	ifconfig >> $logPath
#	echo "IO node rank: $rank " >> $logPath
#	echo "jobID: $jobID" >> $logPath
	mkdir $base/IO-$rank/$jobID
#	touch $base/IO-$rank/$jobID/neighbor
#	touch $base/IO-$rank/$jobID/svr_reg
	rm -fr /tmp/*
	echo $jobID > /tmp/jobID #for worknode to use

        #sleep 10 # wait for work node write IP
#	rm $logPath
#       	mkdir /tmp/neighbors
#       	mkdir /tmp/registers
	/home/dzhao/WorkNodeCollector.sh &
	
#	echo "Done with calling scripts: start" >> $logPath
       
#       neighborFile=`ls /tmp`
#        numNode=`echo $neighborFile |awk -F '_' {'print $2'}`
#        registerFile="Register_$numNode"
#        echo " neighborFile: $neighborFile" >> $logPath
#       echo "registerFile: $registerFile" >> $logPath
        #======================================================================================================



        #echo "IO node start" >> /intrepid-fs0/users/tonglin/persistent/IO_test
        #ifconfig >> /intrepid-fs0/users/tonglin/persistent/IO_test
        #echo "" >> /intrepid-fs0/users/tonglin/persistent/IO_test
fi

