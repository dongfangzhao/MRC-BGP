/************************************************************************************
 * This is a simple test to write and read an float array for PNNL compress project
 *
 * Author: 		DFZ
 * Date:		6/27/2013
 * History:
 * 		6/27/2013: passed on writing and reading on /dev/ram/testfile
 * **********************************************************************************
 */

#include "common.h"

void log_msg(const char *format, ...);
void print_q(fname_q *head);

size_t ELEM_TOTAL = NLAT * NLON * NLAYER;

//char *fname2 = "/dev/shm/mountdir/testfile2";

int main(int argc, char **argv)
{
	/*prepare data*/
//	int rank = 0;
//	float data[NLAT * NLON * NLAYER];
//	int lat, lon, layer;
//	for (lat = 0; lat < NLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++)
//			{
//				data[lat * NLON * NLAYER + lon * NLAYER + layer] =
//						rank * 1000	+ lat * 1 + lon * 0.1 + layer * 0.001;
//			}
//		}
//	}
//	printf("\n Original data: \n");
//	for (lat = 0; lat < NLAT; lat++)
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
	struct timeval tv;
	double t1, t2;

	long iter = 20000; // 200~10MB, 2000~100MB, 20000~1GB

	char fname_scratch[256] = "/home/dzhao/persistent/rootdir/";
	if (argc > 1)
	{
		strcat(fname_scratch, argv[1]);
		strcat(fname_scratch, "/testfile_scratch");
	}

	char fname_log[256] = "/home/dzhao/persistent/rootdir/";
	if (argc > 1)
	{
		strcat(fname_log, argv[1]);
		strcat(fname_log, "/log");
	}

//	char fname[256] = "/dev/shm/mountdir/testfile"; /*this is only for FUSE*/
	char fname[256] = "/home/dzhao/persistent/rootdir/"; /*this is for direct GPFS (multiple dir) */
	if (argc > 1)
	{
		strcat(fname, argv[1]);
		strcat(fname, "/testfile");
	}
	printf("fname = %s, \n",
			fname);

	/*read the oringal data*/
	char *fname_orig = "/dev/shm/temperature.data";
	FILE *data_fp = fopen(fname_orig, "rb");
	if (!data_fp)
	{
		perror("failed to open file");
		return 1;
	}
	float *data = (float*)malloc(sizeof(float) * ELEM_TOTAL);
	fread(data, sizeof(float), ELEM_TOTAL, data_fp);
	fclose(data_fp);

//	/*for big-endian*/
	size_t i;
	for (i = 0; i < ELEM_TOTAL; i++)
	{
		uint32_t *ip = (uint32_t*)&(data[i]);
		*ip = bswap_32(*ip);
	}

	FILE *fp;

	/*write to GPFS directly*/
//
//	gettimeofday(&tv, NULL);
//	t1 = tv.tv_sec + (double)tv.tv_usec / 1000000;;
//	printf("tv.tv_sec = %ld, tv_usec = %ld, \n", tv.tv_sec, tv.tv_usec);
//	fp = fopen(fname_scratch, "wb");
//	for (i = 0; i < iter; i++)
//	{
//		fwrite(data, sizeof(float), ELEM_TOTAL, fp);
////		fseek(fp, 0, SEEK_SET);
//	}
//	fclose(fp);
//	sync();
//
//	gettimeofday(&tv, NULL);
//	t2 = tv.tv_sec + ((double)tv.tv_usec) / 1000000;
//	printf("tv.tv_sec = %ld, tv_usec = %ld, \n", tv.tv_sec, tv.tv_usec);
//	printf("GPFS: t1 = %lf, t2 = %lf, diff = %lf, \n",
//			t1, t2, t2 - t1);

	/*write to file (compressed)*/
//	gettimeofday(&tv, NULL);
//	t1 = tv.tv_sec + (double)tv.tv_usec / 1000000;

		fp = fopen(fname, "wb");
	for (i = 0; i < iter; i++)
	{
		fwrite(data, sizeof(float), ELEM_TOTAL, fp);
	}
		fclose(fp);
		sync();

//	gettimeofday(&tv, NULL);
//	t2 = tv.tv_sec + (double)tv.tv_usec / 1000000;
//
//	FILE *fp_log = fopen(fname_log, "w");
//	fprintf(fp_log, "MRFS %s, t1 = %f, t2 = %f, diff = %f, oh = %f, ratio = %lf, \n",
//			argv[1], t1, t2, t2 - t1, oh_compute, (double)compressed_size / (NLON * NLAYER * sizeof(float)));
//	fclose(fp_log);

	/*read from the compressed file*/

	char *data_reload = (char *)malloc(sizeof(float) * ELEM_TOTAL);
	memset(data_reload, 0, sizeof(float) * ELEM_TOTAL);

		gettimeofday(&tv, NULL);
		t1 = tv.tv_sec + (double)tv.tv_usec / 1000000;
	fp = fopen(fname, "rb");
	for (i = 0; i < iter; i++)
	{
		fread(data_reload, sizeof(float), ELEM_TOTAL, fp); //TODO: read doubled size to be safe
	}
	fclose(fp); /*TODO: for some reason I could not close this FP when directly reading from file ...*/

//	/*calculate the average*/
//	float avg[NLON] = {0};
//	float minf = FLT_MAX, maxf = FLT_MIN;
//	int j;
//	for (i = 0; i < NLON; i++)
//	{
//		for (j = 0; j < NLAYER; j++)
//		{
//			float f = ((float *)data_reload)[i * NLAYER + j];
//			avg[i] += f;
//			if (f < minf)
//				minf = f;
//			if (f > maxf)
//				maxf = f;
//		}
//		avg[i] /= NLAYER;
//	}
//
		gettimeofday(&tv, NULL);
		t2 = tv.tv_sec + (double)tv.tv_usec / 1000000;
		FILE *fp_log = fopen(fname_log, "w");
		fprintf(fp_log, "MRFS %s, t1 = %f, t2 = %f, diff = %f, \n",
				argv[1], t1, t2, t2 - t1);
		fclose(fp_log);


//	printf("\n Reloaded data: \n");
//	for (lat = 0; lat < NLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++) {
//				printf(" %f",
//						((float *)data_reload)[lat * NLON * NLAYER + lon * NLAYER	+ layer]);
//			}
//			printf("\n");
//		}
//	}

//	/**********************************
//	 * test write/read the second file in the cache
//	 * *********************************/
//
//	/*prepare data for the 2nd file*/
//	float data2[NLAT * NLON * NLAYER];
//	for (lat = 0; lat < NLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++)
//			{
//				data2[lat * NLON * NLAYER + lon * NLAYER + layer] =
//						1 + rank * 1000	+ lat * 1 + lon * 0.1 + layer * 0.001;
//			}
//		}
//	}
//	printf("\n 2nd Original data: \n");
//	for (lat = 0; lat < NLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++)
//			{
//				printf(" %f", data2[lat * NLON * NLAYER + lon * NLAYER + layer]);
//			}
//			printf("\n");
//		}
//	}
//
//	/*write to file*/
//	FILE *fp2 = fopen(fname2, "wb");
//	fwrite(data2, sizeof(float), ELEM_TOTAL, fp2);
//	fclose(fp2);
//
//	fp2 = fopen(fname2, "rb");
//	memset(data_reload, 0, sizeof(float) * ELEM_TOTAL);
//	fread(data_reload, sizeof(float), ELEM_TOTAL * 2, fp2); //read doubled size to be safe
//	fclose(fp2); /*TODO: for some reason I could not close this FP when directly reading from file ...*/
//
//	printf("\n 2nd Reloaded data: \n");
//	for (lat = 0; lat < NLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++) {
//				printf(" %f",
//						((float *)data_reload)[lat * NLON * NLAYER + lon * NLAYER	+ layer]);
//			}
//			printf("\n");
//		}
//	}
//
//	/******************************
//	 * test reading the first file again
//	 * **************************************
//	 */
//	FILE *fp3 = fopen(fname, "rb");
//
//	memset(data_reload, 0, sizeof(float) * ELEM_TOTAL);
//
//	fread(data_reload, sizeof(float), ELEM_TOTAL * 2, fp3); //read doubled size to be safe
//	//fclose(fp3); /*TODO: for some reason I could not close this FP when directly reading from file ...*/
//
//	printf("\n Reloaded 1st data: \n");
//	for (lat = 0; lat < NLAT; lat++)
//	{
//		for (lon = 0; lon < NLON; lon++)
//		{
//			for (layer = 0; layer < NLAYER; layer++) {
//				printf(" %f",
//						((float *)data_reload)[lat * NLON * NLAYER + lon * NLAYER	+ layer]);
//			}
//			printf("\n");
//		}
//	}

	/*that's all folks.*/
	return 0;
}
