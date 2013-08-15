cd ./src
make clean

cd ./zht
make clean

cd ../ffsnet
make clean

cd ../udt
make clean

cd ../../
rm fusionfs.log

cd ./test
make clean

cd ./bgp_script
./remove_cobalt_log.sh

echo =========Clean succeed==========
