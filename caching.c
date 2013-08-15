/*************************************************************************************************
 * 	Author: 	DFZ
 * 	Desc:		The caching management for the multi-reference compression algorithm
 * 	History:
 * 		- 6/25/2013:
 * 			- refactor code with new header file common.h, shared with compress.c
 * 			- implemented and tested cache_deque(), data cached as in case 1 on 6/24/2013
 * 				- case 1: read "a1", "a2" and "a3", all returned with the correct values
 * 				- case 2: read "a4", not found in cache
 * 		- 6/24/2013:
 * 			- cache_enque() tested on 3 arrays a1 = a2 = a3 = float[10], caching a1, a2 and a3:
 * 				- case 1: MAX_CACHE = 120, final output queue = a1 -> a2 -> a3
 * 				- case 2: MAX_CACHE = 80, final output queue = a2 -> a3
 *				- case 3: MAX_CACHE = 40, final output queue = a3
 * 		- 6/24/2013: created
 *************************************************************************************************
 */

#include "common.h"

void log_msg(const char *format, ...);
void print_q(fname_q *head);

/**
 * 	Initialize the memory for caching the temporary compressed data
 *
 * 	Input:
 * 		req_size: requested size
 * 		start_pos: returned position where the requested memory starts
 *
 * 	Output:
 * 		status flag
 */
int mem_init(size_t req_size, uint32_t **start_pos)
{
	*start_pos = malloc(sizeof(uint32_t) * req_size);
	assert(*start_pos);

	/*Very important: init the hash table also*/
	hcreate(MAX_HT);

	/*reset the global variables*/
	q_head = NULL;
	cnt = 0;
	oh_compute = 0;
	compressed_size = 0;

	return 0;
}

/**
 * 	Destroy the memory for caching
 *
 * 	Input:
 * 		start_pos: the address from where to be deallocated
 *
 * 	Output:
 * 		status flag
 */
int mem_destroy(uint32_t *start_pos)
{
	free(start_pos);

	return 0;
}

/**
 *	Get the address of the last free entry of the allocated memory
 *
 *	Input:
 *		cache_addr: starting position of the memory cache
 *		ent_addr: returned address of the free entry
 *		avail_size: returned maximally available size of memory
 *
 *	Output:
 *		status flag
 */
int mem_get_free_entry(uint32_t *cache_addr, uint32_t **ent_addr, int *avail_size)
{
	/*assuming global queue is not empty*/
	assert(q_head);

	fname_q *q_tail = q_head->prev;
	ENTRY e, *ep;
	e.key = (char *)q_tail->filename;
	ep = hsearch(e, FIND);
	assert(ep);
	uint32_t *tail_addr = ((mr_file *)ep->data)->addr;

	*ent_addr = tail_addr + ((mr_file *)ep->data)->len;

//	log_msg("d78 %s-%d:  ((mr_file *)ep->data)->len = %x, \n",
//			__FILE__, __LINE__, ((mr_file *)ep->data)->len);

	*avail_size = MAX_CACHE - (*ent_addr - cache_addr) * sizeof(uint32_t);

	return 0;
}

/**
 * 	Insert a new file into the cache when writing the file
 *
 * 	Input:
 * 		cache_addr:	the address of the cache
 * 		cache_size:	the size of the cache
 * 		data_addr:	the address of the file
 * 		data_size:	the size of the data
 * 		fname:		the file name
 * 		n_ref:		number of references
 *
 * 	Output:
 * 		status flag
 */
inline int cache_write(
		uint32_t	*cache_addr,
		size_t		cache_size,
		uint32_t	*data_addr,
		size_t		data_size,
		char		*fname,
		int			n_ref)
{
	int stat = 0;

	if (data_size * sizeof(uint32_t) > cache_size)
	{
		printf("INFO: data too big to fit into cache. \n");
		return 0;
	}

	/*find the appropriate starting address for data, if needed*/
	uint32_t *file_start = cache_addr;
	int avail_size = MAX_CACHE;
	if (q_head)
	{
		stat = mem_get_free_entry(cache_addr, &file_start, &avail_size);
//		log_msg("d78 %s-%d: stat = %d, cache_addr = %x, file_start = %x, avail_size = %d, \n",
//				__FILE__, __LINE__, stat, cache_addr, file_start, avail_size);
	}
	if (stat)
	{
		perror("Cannot find free entry for file \n");
		return 1;
	}
	if (avail_size < data_size * sizeof(uint32_t))
	{
		printf("INFO: Not enough space from the last entry; swipe from the beginning. \n");
		file_start = cache_addr;
	}

	uint32_t *file_end = file_start + data_size - 1;

//	log_msg("d79 %s-%d: cache_addr = %x, file_start = %x, data_size = %d, \n",
//			__FILE__, __LINE__, cache_addr, file_start, data_size);

	/*invalidate dirty entries in hash table and queue*/
	fname_q *cur = q_head;

	if (cur)
	{
		do
		{
			uint32_t *cur_start, *cur_end;
			ENTRY e, *ep;
			e.key = (char*)cur->filename;
			ep = hsearch(e, FIND);

//			log_msg("d79 %s-%d: e.key = %s, ep = %x, ep->data = %x, \n",
//					__FILE__, __LINE__, e.key, ep, ep->data);

			if (ep && ((mr_file*)ep->data)->avail)
			{
				cur_start = ((mr_file *)ep->data)->addr;
				cur_end = cur_start + ((mr_file *)ep->data)->len - 1;

				if (! (file_start > cur_end || file_end < cur_start))
				{
					/*update hashtable*/
//					e.data = NULL;
//					ENTRY *ep2;
//					ep2 = hsearch(e, ENTER); /*<search.h> doesn't support hastable override*/

					((mr_file*)ep->data)->avail = 0;

					/*update queue*/
					if (q_head == cur) {
						printf("INFO: Queue head updated. \n");
						if (q_head->next != q_head)
						{
							q_head = q_head->next;
						}
						else
						{
							q_head = NULL;
							break;
						}
					}
					remque(cur);
				}
			}
			else
			{
				perror("File in queue, but not in hash table");
				return 1;
			}

			cur = cur->next;
		} while (cur != q_head);
	}

	/*copy over the data*/
	memcpy(file_start, data_addr, data_size * sizeof(uint32_t));

	if (DEBUG)
	{
		int i;
		printf("[");
		for (i = 0; i < data_size; i++)
		{
			printf(" %f", *(float *)(file_start + i));
		}
		printf(" ] \n");
	}

	/*insert the new file into the hashtable*/
	ENTRY e_write, *ep;
	mr_file *fp = malloc(sizeof(mr_file));
	fp->addr = file_start;
//	log_msg("d78 %s-%d: data_size = %d, \n",
//				__FILE__, __LINE__, data_size);
	fp->len = data_size;
	fp->n_ref = n_ref;
	fp->avail = 1;

//	/*adjust temp dict and save it to metadata*/
//	size_t file_offset = file_start - cache_head;
//	log_msg("d78 %s-%d: file_offset = %d, \n",
//				__FILE__, __LINE__, file_offset);
//	int i;
//	for (i = 0; i < MAX_REF * 2; i += 2)
//	{
//		dict[i] += file_offset;
//	}

	memcpy(fp->dict, dict, sizeof(dict));

//	log_msg("d78 %s-%d: (f->dict)[0] = %d, (f->dict)[1] = %d, \n",
//			__FILE__, __LINE__, (fp->dict)[0], (fp->dict)[1]);
//	log_msg("d78 %s-%d: (f->dict)[2] = %d, (f->dict)[3] = %d, \n",
//			__FILE__, __LINE__, (fp->dict)[2], (fp->dict)[3]);

	strcpy(entry_key[cnt], fname);
	e_write.key = entry_key[cnt];
	e_write.data = fp;
	ep = hsearch(e_write, ENTER);

//	log_msg("d78 %s-%d: cnt = %d, e_write.key = %s, e_write.data = %x, ep = %x, \n",
//			__FILE__, __LINE__, cnt, e_write.key, e_write.data, ep);


	ENTRY e_test_write, *ep_test_write;
	e_test_write.key = entry_key[cnt];
	ep_test_write = hsearch(e_test_write, FIND);

//	log_msg("d78 %s-%d: cnt = %d, dep_test_write->key = %s, ep_test_write->data = %x, ep_test_write = %x, \n",
//			__FILE__, __LINE__, cnt, ep_test_write->key, ep_test_write->data, ep_test_write);

	//mr_file *datap = (mr_file*)(ep_test_write->data);
//	log_msg("d78 %s-%d: datap->len = %d, (datap->dict)[0] = %d, (datap->dict)[1] = %d, \n",
//			__FILE__, __LINE__, datap->len, (datap->dict)[0], (datap->dict)[1]);

	cnt++; /*TODO: why do I need to use different key locations?*/

	/*insert the new file into the queue*/
	fname_q *q_tail, *new_elem = malloc(sizeof(fname_q));
	if (q_head)
	{
		q_tail = q_head->prev;
		strcpy(new_elem->filename, fname);
		insque(new_elem, q_tail);
	}
	else
	{
		q_head = new_elem;
		strcpy(q_head->filename, fname);
		q_head->next = q_head;
		q_head->prev = q_head;
	}

    /*DFZ test ENTRY*/
//    char *key_str = "/home/dongfang/pnnl2013/rootdir/testfile1";
//    char *key_val = "testvalue";
//    ENTRY e1, e3, *ep3;
//    e1.key = key_str;
//    e1.data = key_val;
//    int ret = hsearch(e1, ENTER);
//    log_msg("DEBUG73 %s-%d: e1.key = %s, ret = %x, \n",
//    		__FILE__, __LINE__, e1.key, ret);
//    e3.key = key_str;
//    ep3 = hsearch(e3, FIND);
//    if (ep3)
//    	log_msg("DEBUG73 %s-%d: ep3->key = %s, ep3->data = %s, ep3 = %x, \n",
//    		__FILE__, __LINE__, ep3->key, (char *)(ep3->data), ep3);

	print_q(q_head);

	return 0;
}

/**
 * 	Read a file from cache when it is available
 *
 * 	Input:
 * 		fname:	file name to (possibly) read from cache
 *		data:	returned pointer to the file data
 *		fsize:	returned file size
 * 	Output:
 * 		status flag
 *
 * 	NOTE: for now, reading cache does not change the priority queue
 */
inline int cache_read(char *fname, uint32_t **data, size_t *f_size)
{
    /*DFZ test ENTRY*/
//	ENTRY e2, *ep2;
//    char *key_str = "/home/dongfang/pnnl2013/rootdir/testfile1";
//    e2.key = key_str;
//    ep2 = hsearch(e2, FIND);
//    if (ep2)
//    log_msg("DEBUG73 %s-%d: ep2->key = %s, ep2->data = %s, ep2 = %x, \n",
//    		__FILE__, __LINE__, ep2->key, (char *)(ep2->data), ep2);

	ENTRY e_test_write, *ep_test_write;
	strcpy(entry_key[cnt], fname);
	e_test_write.key = entry_key[cnt];
	ep_test_write = hsearch(e_test_write, FIND);

    /*retrieve the metadata from the hash table*/
    ENTRY e_read, *ep;
	e_read.key = fname;
	ep = hsearch(e_read, FIND);

	if (!ep)
	{
		*data = NULL;
		return 0;
	}

	mr_file *f = (mr_file *)ep->data;
	uint32_t *f_addr = f->addr;
	*f_size = f->len;
	memset(dict, 0, sizeof(dict));
	memcpy(dict, f->dict, sizeof(dict)); /*load the working file into temp dict*/

//	log_msg("d78 %s-%d: (f->dict)[0] = %d, (f->dict)[1] = %d, \n",
//			__FILE__, __LINE__, (f->dict)[0], (f->dict)[1]);
//	log_msg("d78 %s-%d: (f->dict)[2] = %d, (f->dict)[3] = %d, \n",
//			__FILE__, __LINE__, (f->dict)[2], (f->dict)[3]);

	/*copy over the file content*/
	if (f->avail)
	{
		*data = malloc(sizeof(uint32_t) * (*f_size));
		memcpy(*data, f_addr, sizeof(uint32_t) * (*f_size));
	}
	else
	{
		*data = NULL;
	}

	return 0;
}

void print_q(fname_q *head)
{
	fname_q *cur = head;
	if (!cur)
	{
//		log_msg("INFO: Global queue does not exist. \n");
		return;
	}

//	log_msg("\n Filename queue: \n");
	do
	{
//		log_msg("-> %s ", cur->filename);
		cur = cur->next;
	} while (cur != head);
//	log_msg("\n");

	if (DEBUG) /*reverse printing the queue*/
	{
	//	do
	//	{
	//		printf("<- %s ", cur->filename);
	//		cur = cur->prev;
	//	} while (cur != head);
	//
	//	printf("\n");
	}
}

void test_caching()
{
	if (DEBUG) /*print simple string queue*/
	{
//		char *fname1 = "file1";
//		char *fname2 = "file2";
//		char *fname3 = "file3";
//
//		fname_q f1, f2, f3;
//		strcpy(f1.filename, fname1);
//		strcpy(f2.filename, fname2);
//		strcpy(f3.filename, fname3);
//		f1.next = &f1;
//		f1.prev = &f1;
//		insque(&f2, &f1);
//		insque(&f3, &f2);
//
//		print_q(&f1);
	}

	/********************************************
	 * prepare data
	 * *******************************************
	 */
	float a1[10] = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9};
	float a2[10] = {2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9};
	float a3[10] = {3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9};

	int i;
	printf("a1 = [");
	for (i = 0; i < 10; i++)
	{
		printf(" %g", a1[i]);
	}
	printf(" ] \n");

	printf("a2 = [");
	for (i = 0; i < 10; i++)
	{
		printf(" %g", a2[i]);
	}
	printf(" ] \n");

	printf("a3 = [");
	for (i = 0; i < 10; i++)
	{
		printf(" %g", a3[i]);
	}
	printf(" ] \n");

	/****************************************
	 * test cache_write
	 * **************************************
	 */
	uint32_t *cache_head;
	mem_init(MAX_CACHE, &cache_head);

	int stat;
	stat = cache_write(cache_head, MAX_CACHE, (uint32_t *)a1, 10, "a1", 1);
	assert(!stat);
	print_q(q_head);

	stat = cache_write(cache_head, MAX_CACHE, (uint32_t *)a2, 10, "a2", 2);
	assert(!stat);
	print_q(q_head);

	stat = cache_write(cache_head, MAX_CACHE, (uint32_t *)a3, 10, "a3", 3);
	assert(!stat);
	print_q(q_head);

	/****************************************
	 * test cache_read
	 * **************************************
	 */
	uint32_t *data;
	size_t f_size;

	stat = cache_read("a1", &data, &f_size);
	assert(!stat);
	if (data)
	{
		printf("a1 = [");
		for (i = 0; i < f_size; i++)
		{
			printf(" %f", *(float *)(data + i));
		}
		printf(" ] \n");
	}

	stat = cache_read("a2", &data, &f_size);
	assert(!stat);
	if (data)
	{
		printf("a2 = [");
		for (i = 0; i < f_size; i++)
		{
			printf(" %f", *(float *)(data + i));
		}
		printf(" ] \n");
	}

	stat = cache_read("a3", &data, &f_size);
	assert(!stat);
	if (data)
	{
		printf("a3 = [");
		for (i = 0; i < f_size; i++)
		{
			printf(" %f", *(float *)(data + i));
		}
		printf(" ] \n");
	}

	stat = cache_read("a4", &data, &f_size);
	assert(!stat);
	if (data)
	{
		printf("a4 = [");
		for (i = 0; i < f_size; i++)
		{
			printf(" %f", *(float *)(data + i));
		}
		printf(" ] \n");
	}

	mem_destroy(cache_head);
}

void test_hsearch()
{
	printf("testing hsearch() \n");
	hcreate(1024);
	char *key = "fname";
	mr_file f;
	f.addr = NULL;
	f.len = 10;
	f.n_ref = 2;
	void *val = &f;
	ENTRY e, *ep;
	e.key = key;
	e.data = val;
	hsearch(e, ENTER);

	ep = hsearch(e, FIND);
	printf("((mr_file *)ep->data)->len = %d \n", (int)((mr_file *)ep->data)->len);
}

void test_float_uint32()
{
	float f = 1.0;
	uint32_t *i = (uint32_t *)&f;
	float f2 = *(float *)i;

	printf("sizeof(float) = %d, sizeof(uint32_t) = %d, f = %f, i = %d, f2 = %f \n",
			(int)sizeof(float), (int)sizeof(uint32_t), f, (int)*i, f2);
}

//int main()
//{
//	test_hsearch();
//	test_float_uint32();
//	test_caching();
//
//	return 0;
//}
