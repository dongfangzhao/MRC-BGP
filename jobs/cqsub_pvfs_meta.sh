rm ~/pvfs/result
rm -rf ~/pvfs/pvfs_tmp
mkdir ~/pvfs/pvfs_tmp
cqsub -p FusionFS -k zepto-vn-eval -n 16 -t 60 pvfs_meta.sh
