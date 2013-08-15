numNode=256
t=60
rm -rf /home/dzhao/persistent/rootdir/*
cqsub -p FusionFS -k zepto-vn-eval -n $numNode -t $t bbfs.sh 
