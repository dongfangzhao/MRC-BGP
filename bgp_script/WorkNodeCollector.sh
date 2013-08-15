#!/bin/sh
# Collect neighbor on IO node, write to a file on GPFS when it's ready.
# Each IO node write to one file, use IO node hostName as file name
#Work node date collect
jobID=$(echo $ZOID_JOB_ENV | sed 's/^.*COBALT_JOBID=\([^:]*\).*$/\1/')

logPath="/intrepid-fs0/users/dzhao/persistent/IO.$jobID"
base="/intrepid-fs0/users/dzhao/persistent/staging"
secondTry="/tmp/NeighborSecondTry"
#echo "Worknode collector running..." >> $logPath
	mkdir /tmp/neighbors
	mkdir /tmp/results
	rank=$(grep BG_PSETNUM /proc/personality.sh | cut -d '=' -f 2)
	numIO=$(grep BG_NUMPSETS /proc/personality.sh | cut -d '=' -f 2)
#	echo "$numIO" >> $logPath

	sleep 10 # wait for work node make a correct file name so get the numNode	
IOnodeName=`hostname`
numInstance=1 #=`cat /home/dzhao/NUM_INST/$jobID.ins`
numNode=$(($numIO * 64)) #$(($numIO * 64 * $numInstance ))
neighborFile="neighbor_$numNode"

#	echo "neighborFile: neighbor_$numNode" >> $logPath
        #`echo $neighborFile |awk -F '_' {'print $2'}`
#regFile="reg_$numNode"
regFile="reg"
	
	nLine=0
        #Check if all work nodes are started
	numLine=$(( 64 * $numInstance ))
        while [[ "$nLine" != "$numLine" ]]
                do
                sleep 5
                nLine=`wc -l /tmp/neighbors/$neighborFile | awk {'print $1'} `
#		echo "Within  IO node-$rank: Neighbor: --------------should be $numLine, nLine = $nLine" >> $logPath
        done #done collect 64 worknode IP

        cp /tmp/neighbors/$neighborFile $base/IO-$rank/$jobID/.
	#Collected from this IO node, using unique filename	

	i=0
#	echo "start merging neighbor list" >> $logPath
	#merge neighbor list
	while [ "$i" != "$numIO" ]
	do		
			
			
        		if [ -a $base/IO-$i/$jobID/$neighborFile ]; then #if file exists
                		#nLine=`wc -l ./IO-$numIO/neighbor | awk {'print $1'}`
                		#total=$(($total + $nLine))
				cat $base/IO-$i/$jobID/$neighborFile >> /tmp/neighbors/listTMP
				nLine=`wc -l /tmp/neighbors/listTMP | awk {'print $1'} `
				$((i++))
			else
				sleep 2
				#echo "$base/IO-$numIO/$jobID/neighbor" >> $secondTry
        		fi
			
        		
	done

	sleep 10
	
#	while read line 
#	do
#		echo "second try with $line" >> $logPath
#		cat $line >> /tmp/neighbors/listTMP
#	done < $secondTry
	 
        rm /tmp/neighbors/$neighborFile
	mv /tmp/neighbors/listTMP /tmp/neighbors/$neighborFile  #update complete neighbor list

#======================================Neighbor List Collecting finished==================================================================
	sleep 5
###########Check server start on IO node, don't need to collect ###############
	




 numLine=$(( 64 * $numInstance ))
        while [[ "$nLine" != "$numLine" ]]
                do
                sleep 1
                nLine=`wc -l /tmp/$regFile | awk {'print $1'} `
#               echo "Within 1 IO node : Neighbor: --------------should be $numLine, nLine = $nLine" >> $logPath
        done #done collect 64 worknode IP

        cp /tmp/$regFile $base/IO-$rank/$jobID/.
        #Collected from this IO node, using unique filename

        i=0

        #merge reg list
        while [ "$i" != "$numIO" ]
        do


                        if [ -a $base/IO-$i/$jobID/$regFile ]; then #if file exists
                                #total=$(($total + $nLine))
                                cat $base/IO-$i/$jobID/$regFile >> /tmp/regTMP
                                nLine=`wc -l /tmp/regTMP | awk {'print $1'} `
                                $((i++))
                        else
                                sleep 1
                                #echo "$base/IO-$numIO/$jobID/neighbor" >> $secondTry
                        fi

        done
	
	rm /tmp/$regFile
	mv /tmp/regTMP /tmp/$regFile
	































#	nLine=0
        #Check if all servers are started
#        while [[ "$nLine" != "64" ]]
#       do
#                sleep 5
#                nLine=`wc -l /tmp/registers/$registerFile | awk {'print $1'} `
#		echo "Within 1 IO node : registers: --------------should be 64, nLine = $nLine" >> $logPath
#        done #done collect 64 worknode IP

#        cp /tmp/registers/$registerFile /intrepid-fs0/users/tonglin/persistent/RegisterD_$numNode/$IOnodeName
        #Collected from this IO node, using unique filename     


#        nLine=0
#        while [[ "$nLine" != "$numNode" ]]
#        do
#                sleep 5
#                nLine=`wc -l /intrepid-fs0/users/tonglin/persistent/RegisterD_$numNode/* |grep total | awk {'print $1'} `
#		echo "All registers: --------------should be $numNode, nLine = $nLine"
#        done # done write from IO nodes to GPFS

#        cat /intrepid-fs0/users/tonglin/persistent/RegisterD_$numNode/* > /tmp/registers/listTMP
#        rm /tmp/registers/$registerFile
#        mv /tmp/registers/listTMP /tmp/registers/$registerFile  #update complete register list


