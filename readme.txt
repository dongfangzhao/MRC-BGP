Author:	DFZ
History:
	7/22/2013:
		- Done with data write, i.e. compression.
	7/19/2013: 
		- everything works fine; start testing performance.
		- fixed the decompression loss
	7/18/2013: FUSE doesn't fully recover temperature data for some reason.
	7/17/2013: FUSE doesn't work for large (>64KB?) data. It's because FUSE handles only the max. system block.
	7/12/2013: 
		- Tested on BG/P with real climate data; compression ratio ~72%; #ref points don't affect much
	7/11/2013:
		- Fixed big-endian problem for IBM PowerPC
	7/9/2013: 
		- Tested caching multiple (i.e. 2) files, when cache size is large enough to hold all
		- Tested caching multiple files when memory is limited:
			- write file1 into cache
			- read file1 from cache
			- write file2 into cache (file1 evicted out of cache)
			- read file2 from cache
			- read file1 from disk
		- Major modifications to hash tables: instead of voiding the value, adding a new bit
			indicating it's out of cache. That is, never void a hashtable entry; this is because
			the hashtable contains the metadata information
	7/8/2013: 
		- Done: dict[] is only fine for one single file; need to extended for multi files; 
			added it to the hashtable
		- Issue: something wrong with <search.h>. The key seems to be only referring to
			a (globally) constant. <search.h> needs to be replaced by a stable hashtable
			implementation. Details, see Line-244 in File caching.c
	7/3/2013: 
		- Passed test case for partial read: start_pos = 20, length = 20. (see Line 466 in bbfs.c)
		- Issue(3) on 7/1/2013 is fixed.
			- ENTRY indeed works fine for this FUSE-based program. The tricky part was
				that the entry.key should be pointed to a globally accessible string variable.
				I am still unsure why this is happening, but it works now.
		- Issue(1) on 7/1/2013 is fixed. You can fclose() now.		
	7/2/2013: fixed issue 2) on 7/1/2013.
	7/1/2013:	All right, this program is getting big since integrating with FUSE.
		- Lines of code: 3056 
		- Passed basic FUSE test case on fusion.cs.iit.edu: write float[40] to the filesystem and recover the file, 
			see test_simple_io.c
		- Issues:
			- 1) test_simple_io.c: why would the second fclose() return a segmentation fault? See "TODO"
			- 2) FIXED. compress.c: the ref_addr in global dict[] should contain the global cache offset, 
					rather than the local compressed[] array. See "TODO" in the file.
			- 3) ENTRY might not work for multi processes, which might be the case for FUSE. 
					Need to do a simple test for inter-process ENTRY variable(e.g. read after write).
					If it does not work, consider using C++ STL.
	6/26/2013: integrated initially, no syntactic error on BlueGene/P
