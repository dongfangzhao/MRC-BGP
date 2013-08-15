rm ~/persistent/result
rm ~/persistent/neighbor
cqsub -p FusionFS -k zepto-vn-eval -n 16 -t 60 fusionfs.sh
