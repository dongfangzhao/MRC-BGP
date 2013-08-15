/*************************************************************
 *	Author:		DFZ
 *	Desc:		Wrapper of compress.c and caching.c
 *	History:
 *		6/26/2013: integrate with FUSE on BlueGene/P
 *		6/25/2013: test_all() added, for integrated testing
 *		6/25/2013: created
 *************************************************************
 */
#include "common.h"

/**
 * 	The first integrated test for (de)compress and caching. It compressed a 4*10 float array,
 *  wrote it to cache, reloaded it from cache, and finally decompressed it. The final output
 *  should be identical to the original array.
 */
void test_all()
{
//	int i = 0x0fffff10;
//	printf("i = %x, *(char*)&i = %x, *((char*)&i+1) = %x, *((char*)&i+2) = %x, *((char*)&i+3) = %x, \n",
//			i, *(char*)&i, *((char*)&i+1), *((char*)&i+2), *((char*)&i+3));
//
//	i <<= 9;
//	printf("i = %x, *(char*)&i = %x, *((char*)&i+1) = %x, *((char*)&i+2) = %x, *((char*)&i+3) = %x, \n",
//			i, *(char*)&i, *((char*)&i+1), *((char*)&i+2), *((char*)&i+3));


//	return;

	/*prepare data*/
//	#define NLAT 1 /*NLAT is not used for now, except for allocating memory for arrays*/
//	#define NLON 150*80
//	#define NLAYER 80
	int lat, lon, layer;

	/*for synthetic data*/
//	int rank = 0;
//	float data[NLAT * NLON * NLAYER];
//	for (lat = 0; lat < NLAT; lat++)
//		for (lon = 0; lon < NLON; lon++)
//			for (layer = 0; layer < NLAYER; layer++)
//			{
//				data[lat * NLON * NLAYER + lon * NLAYER + layer] =
//						rank * 1000	+ lat * 1 + lon * 0.1 + layer * 0.001;
//			}

	char *fname = "./temperature.data";
	FILE *data_fp = fopen(fname, "rb");
	if (!data_fp)
	{
		perror("failed to open file");
		return;
	}
	float *data = (float*)malloc(sizeof(float) * NLAT * NLON * NLAYER);
	fread(data, sizeof(float), NLAT * NLON * NLAYER, data_fp);
	fclose(data_fp);

	/*for big-endian*/
	int i;
	for (i = 0; i < NLAT * NLON * NLAYER; i++)
	{
		uint32_t *ip = (uint32_t*)&(data[i]);
		*ip = bswap_32(*ip);
	}

	float *compressedData = (float*)malloc(sizeof(float) * NLAT * NLON * NLAYER);
	float *unCompressedData = (float*)malloc(sizeof(float) * NLAT * NLON * NLAYER);

//	printf("\n Original data: \n");
//	for (lat = 0; lat < MAXLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++)
//			{
//				printf(" %f", data[lat * NLON * NLAYER + lon * NLAYER + layer]);
//			}
//			printf("\n");
//		}
//	}

	/*compress data*/
	size_t ret = encode_multiref(data, NLON * NLAYER * sizeof(float),
			compressedData,	NLON * NLAYER * sizeof(float), NLAYER, NREF);

	printf("Compressed size = %ld bytes. \n", ret);

	/*save the compressed data*/
	FILE *fp_comp = fopen("./temperature.compressed", "wb");
	fwrite(compressedData, sizeof(char), ret, fp_comp);
	fclose(fp_comp);

//	/*print the compressed data*/
//    int i;
//    for (i = 0; i < 60; i++)
//    {
//    	printf("INFO %s-%d: compressedData[%d] = %x \n", __FILE__, __LINE__, i, *((uint32_t *)compressedData + i));
//    }

	/*write to cache*/
	uint32_t *cache_head;
	mem_init(MAX_CACHE, &cache_head);
	int stat = cache_write(cache_head, MAX_CACHE, (uint32_t *)compressedData,
			(ret + 3) / 4, "testfile", NREF);
	assert(!stat);
	if (DEBUG)
		print_q(q_head);

	/*reload from cache*/
	uint32_t *data_reload;
	size_t f_size;
	stat = cache_read("testfile", &data_reload, &f_size);
	assert(!stat);
	if (DEBUG)
	{
		if (data_reload)
		{
			printf("INFO: %d compressed words loaded successfully. \n", f_size);
		}
		else
		{
			printf("INFO: not found in cache. \n");
		}
	}

	/*decompress data*/
	ret = decode_multiref(data_reload, NLON * NLAYER * sizeof(float),
			unCompressedData, NLON * NLAYER * sizeof(float),
			NLAYER, NREF,
			0, NLON * NLAYER); /*change these 2 parameters for: start position, length*/

//	printf("\n Restored data: \n");
//	for (lat = 0; lat < MAXLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++) {
//				printf(" %f",
//						unCompressedData[lat * NLON * NLAYER + lon * NLAYER	+ layer]);
//			}
//			printf("\n");
//		}
//	}

	/*cleanup cache*/
	mem_destroy(cache_head);

	printf("\n");
}

int main()
{
	test_all();
	return 0;
}
