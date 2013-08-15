if [ -z $1 ]
then
	echo "Missing number of nodes"
	exit
fi

rm ~/persistent/neighbor

cqsub -p FusionFS -k zepto-vn-eval -n $1 -t 60 pvfs2.sh $1
