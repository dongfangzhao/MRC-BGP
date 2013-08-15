Author: dongfang.zhao@hawk.iit.edu

Update history:
	05/09/2013: adding staging bootstrap and result collection. Scaled up to 16K nodes with 2.5TB/s with this version. 
				I expect about 5TB/s at 32K-node, however we've run out of hours on Intrepid.
	09/15/2012: some ad-hoc changes to fusionFS for metadata benchmark for ZHT-IPDPS submission
	09/08/2012: fusionFS running fine on a single work node on BluegeneP. IOZone result: read 48 MB/s, write 23 MB/s
	08/30/2012: initial checked in, compiled and mounted succesfully; 'ls' is not functioning, nothing else is tested

How to install fusionfs:
	0) This needs to be updated for BGP.
	1) Make sure all dependent libraries are specified, e.g. echo $LD_LIBRARY_PATH 
		:/usr/local/lib:/home/dongfang/fusionFS/src/ffsnet/:/home/dongfang/fusionFS/src/udt4/src
	2) Install FUSE 2.8 or later for your Linux distribution
	3) Install Google Protocol Buffer 
	4) Go to ./src/zht/ and run Makefile
	5) Go to ./src/udt/ and run Makefile
	6) Go to ./src/ffsnet/, run Makefile
	7) Go to ./src, run Makefile
	8) You can use ./compileAll if you are sure each individual Makefile works fine
	9) ./cleanALL to `make clean` everything

How to use fusionfs:
	0) This needs to be udpated for BGP.
	1) ./clearRootDir to cleanup your scratch data
	2) ./start_service to start the backup services
	3) ./start to run fusionfs
	4) ./stop to stop fusionfs
	5) ./stop_service to stop all services

How to test fusionfs with IOZone:
	1) It's trivial to run IOZone on the local node
	2) To test multiple nodes:
		2.1) Create a file on node A, say `touch testfile1`
		2.2) On node B, run: `iozone -awpe -s 1m -r 4k -i 0 -i 1 -i 2 -f testfile1`
		2.3) Now check back in node A and verify testfile1 has 1MB by `ls testfile1 -lh`

How to test fusionfs with IOR:
	1) Install openmpi
	2) Install IOR
	3) For single node test, just run `fusion_mount/IOR`
	4) For multiple node test:
		4.1) Create a file of list of hosts, YOUR_HOSTFILE e.g.
			hec-01
			hec-02
			...
		4.2) On any node, run `fusion_mount/mpiexec -hostfile YOUR_HOSTFILE IOR -F`. This is
			to test the aggregate bandwidth with all local read/write for each process.
		4.3) Run `fusion_mount/mpiexec -hostfile YOUR_HOSTFILE IOR -F -C -k`. This is to test
			local write and remote read in a round-robin fashion for the aggregate bandwidth.
			That is, file_i is written on node_i and then will be read by node_(i+1). See
			option -C from the IOR User Manual for more information. '-k' is required here, 
			or you will encounter node_i and node_i+1 are trying to remove the same file 
			concurrently, which is a write-write conflict that FusionFS doesn't support for now.
	IMPORTANT NOTE: In IOR, when it claims it "reads" a file, it indeed opens the file with	mode 02. 
		Mode 02 means read AND write. Therefore, write-write locks are expected even if you
		only conduct read-only experiments with IOR.
	
Note:
    *If you make your desktop run ffsnetd service, please make sure no firewall is blocking this service from outside request.
		- In Fedora, you can turn off firewall by `sudo service iptables stop`
		- Note that there's no firewall inside IBM Bluegene

