/****************************************************
 * 	Header file for:
 * 		- compress.c
 * 		- caching.c
 *
 * 	Author: DFZ
 * **************************************************
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <search.h>
#include <byteswap.h>
#include <time.h>
#include <sys/time.h>
#include <float.h>

#define DEBUG 0			/*change this to enable/disable the debug information*/
#define MAX_CACHE 128*1024*1024 	/*maximal bytes of cache*/ /*change this for different cache size*/
#define MAX_HT 1024		/*maximal hashtable entries*/
#define MAX_REF 100 	/*max number of reference points*/
#define MAXLAT 1		/*1 for now*/

#define NLAT 1 			/*NLAT is not used for now*/
#define NLON 80*2 		/*make sure NLON * NLAYER < 64K*/
#define NLAYER 80
#define NREF 1

/**
 * hash table entry for the multi-ref file (i.e. metadata)
 */
typedef struct _mr_file
{
	size_t		n_ref;	/*number of references*/
	uint32_t	*addr;	/*address of the file*/
	size_t		len;	/*length of the file*/
	long 		dict[MAX_REF * 2];	/*record the positions of compressed data within a file:
	 	 	 	 	 	 	 	 	  <word_offset, bit_offset> */
	int			avail;	/*boolean: 1 - in cache; 0 - not in cache*/
} mr_file;

/**
 * Element structure of the global queue of caching files
 */
typedef struct _fname_q
{
	struct 	_fname_q *next;
	struct 	_fname_q *prev;
	char 	filename[256];
} fname_q;

/**
 * global variables
 */
char entry_key[256][256];		/*the string space for entry key in hsearch()*/
int cnt; /*global counter*/
fname_q *q_head;			/*queue for list of all cached filenames*/
long dict[MAX_REF * 2];		/*temporary map <filename, word_addr + bit_offset> for (de)compress*/
uint32_t *cache_head;
double oh_compute;			/*the computing overhead for compression*/
long compressed_size;		/*compressed size*/
