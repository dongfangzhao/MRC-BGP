/**
 * 6/25/2013
 * 		- DFZ: refactor code
 * 6/24/2013
 * 		- DFZ: passed the following cases for the 10*10 arrays
 * 			- ref = 2, start = 0, len = 100
 * 			- ref = 2, start = 50, len = 50
 * 			- ref = 5, start = 40, len = 20
 *			- ref = 5, start = 41, len = 20 // should be 3 lines 0.4..., 0.5..., 0.6...
 *			- ref = 5, start = 39, len = 50
 *		- DFZ: add cache management

 * 6/20/2013, DFZ: passed the following test cases for multiple references:
 * 					- 2 ref points, 40 numbers compressed
 * 						* decompress from index 0, count = 40
 * 						* decompress from index 0, count = 20
 * 						* decompress from index 20, count = 20
 *
 * 6/19/2013, DFZ: add support of multiple reference points
 * 6/18/2013, DFZ: fix bugs; update logic and comments
 * DFZ: this is a new file by Jian.
 */

#include "common.h"
#define HAVE_DECL___BUILTIN_BSWAP64 1
void log_msg(const char *format, ...);

/**
 * return the leading zeros of input x
 */
inline int nlz11(unsigned x){
        return (x==0)?32:__builtin_clz(x);
}
/* //not working on BlueGene/P
inline int nlz11(unsigned x)
{
	if (x == 0)
		return 32;
	else
	{
		int index;
		asm
		(
				"bsr  %1, %0;"
				:"=r" (index)
				:"r"(x)
		);

		return 31 - index;
	}
}
*/

/**
 * Parameters
 * 	<*cur>: the word address
 * 	<*position>: the offset within *cur
 * 	<bits>: the raw number (either #0's or the tailing difference)
 * 	<NBit>: the max. number of bits allowed to store <bits>
 */
inline void packBits(uint32_t ** cur, int * position, uint32_t bits, int NBit)
{
	if (NBit == 0)
		return;

	uint64_t shiftedBits = bits;
	shiftedBits = shiftedBits << (*position); //DFZ: i think this is enough

	/*begin changes to big-endian systems*/
//	shiftedBits = bswap_64(shiftedBits);
	if ((*position + NBit) > 31)
	{
		**cur |= *((uint32_t *)&shiftedBits + 1);
		(*cur)++;
		**cur |= *((uint32_t *)&shiftedBits);
	}
	else
	{
		**cur |= *((uint32_t *)&shiftedBits + 1);
	}
	/*end changes to big-endian systems*/

//	/*the following only works for little endians*/
//	*((uint64_t *) (*cur)) |= shiftedBits;
//
//	/* sometimes the compressed data is even larger than the original,
//	 * so we need to carry the (*cur) and update the (*positoin) */
//	if ((*position + NBit) > 31)
//	{
//		(*cur)++; /*so you assume the machine is little-endian*/
//	}

	*position = (*position + NBit) % 32;
}

/**
 * Return the stored value of a bit stream (defined by *cur + position);
 * NBit is the max. number of bits to store the value
 */
inline uint32_t unpackBits(uint32_t ** cur, int * position, int NBit)
{
	if (NBit == 0)
		return 0;

	/*begin mod for big-endian systems*/
	int32_t bits = 0;
	if ((*position + NBit) > 31)
	{
		int n_high = (*position + NBit) - 32;
		uint32_t high_bits = (*(*cur + 1) << (32 - n_high)) >> (32 - n_high);
		int n_low = 32 - *position;
		uint32_t low_bits = **cur >> *position;
		bits = (high_bits << n_low) | low_bits;

		(*cur)++;
	}
	else
	{
		bits = ((**cur) << (32 - *position - NBit)) >> (32 - NBit);
	}
	/*end mod for big-endian systems*/

	/*retrieve the stored value, only works for little-endian*/
//	uint64_t oldBits = *(uint64_t *) *cur;
//	if (DEBUG)
//		printf("oldBits = %lx, NBit = %d, *position = %d, \n", (long unsigned int)oldBits, NBit, *position);
//
//	uint64_t bits = (oldBits << (64 - *position - NBit)) >> (64 - NBit);
//
//	/*update the cursor*/
//	if ((*position + NBit) > 31)
//		(*cur)++;


	*position = (*position + NBit) % 32;
	return (uint32_t)bits;
}

/*
 * Compress curValue based on the base value of prevValue and the XOR delta
 * Input: <prevValue>, <curValue>
 * Output: <**cur>, <*position>
 */
inline void packValue(uint32_t prevValue, uint32_t curValue, uint32_t ** cur, int * position)
{

	register uint32_t diff = prevValue ^ curValue;
//	printf("d712 %s-%d: prevValue = %x or %f, curValue = %x or %f, diff = %x, \n",
//			__FILE__, __LINE__, prevValue, *(float *)&prevValue, curValue, *(float *)&curValue, diff);
	register uint32_t NZero = nlz11(diff);

	if (DEBUG)
	{
		printf("diff = %x, NZero = %d, \n", diff, NZero);
	}

//	printf("NZero = %d \n", NZero);

	/*We can only compress max. 31 0's */
	if (NZero == 32)
		NZero = 31;

	/*We want to compress NZero 0's into lower 5 bits (starting from pos 0)*/
	packBits(cur, position, NZero, 5);

//	printf("d710: **cur = %lx, \n", (long unsigned int)*(uint64_t *)*cur);

	/*We then save the remaining bits on the following bits of the lower 5 bits of leading 0's*/
	register char NRem;
	if (NZero == 31) /*again, special case for 31 leading 0's*/
		NRem = 1;
	else
		NRem = 32 - NZero;
	packBits(cur, position, diff, NRem);

//	printf("d710 %s-%d: *(uint64_t*)*cur = %x \n", __FILE__, __LINE__, *(uint64_t *)*cur);
}

/**
 * Given the base value <prevValue> and the XOR delta (in <*cur> + position), return the original val
 */
inline uint32_t unpackValue(uint32_t prevValue, uint32_t ** cur, int * position)
{
	int lz = unpackBits(cur, position, 5);
	if (DEBUG)
		printf("lz = %d \n", lz);

	int NRem = 1; /*for the special case of 31 leading zeros*/
	if (lz < 31)
		NRem = 32 - lz;

	int second_unpack_val = unpackBits(cur, position, NRem);

	if (DEBUG)
	{
		printf("second_unpack_val = %d \n", second_unpack_val);
		printf("prevValue = %x, %f, \n", prevValue, *(float *)&prevValue);
	}

	uint32_t curValue = prevValue ^ second_unpack_val;

	if (DEBUG)
		printf("curValue = %x, %f \n", curValue, *(float *)&curValue);

	return curValue;
}		  

/**
 * Compress an 2-d array
 * Return value is the address of the last compressed word
 *
 * Potential issue: the write buffer might be larger than the read buffer
 */
size_t encode_xor(void *read_buffer, size_t read_buffer_size,
		void *write_buffer, size_t write_buffer_size, size_t n_layers)
{
	assert(read_buffer_size != 0);
	uint32_t *l_read_buffer = (uint32_t *) read_buffer;
	uint32_t *l_write_buffer = (uint32_t *) write_buffer;

	size_t read_buffer_count = read_buffer_size / sizeof(*l_read_buffer);
	assert(((read_buffer_count % n_layers) == 0));

	/*setup the base values*/
	memcpy(l_write_buffer, l_read_buffer, sizeof(*l_read_buffer) * n_layers);
	size_t rows = read_buffer_count / n_layers;
	memset(l_write_buffer + n_layers, 0,
			(rows - 1) * n_layers * sizeof(*l_write_buffer));

	uint32_t * cur = l_write_buffer + n_layers;
	int bit_position = 0;
	register size_t i, j;
	for (i = 1; i < rows; i++)
	{
		for (j = 0; j < n_layers; j++)
		{
			uint32_t prevValue = l_read_buffer[(i - 1) * n_layers + j];
			uint32_t curValue = l_read_buffer[i * n_layers + j];

			packValue(prevValue, curValue, &cur, &bit_position);
		}
	}

	// so you are returning the address of the very last word
	return (cur - l_write_buffer + 1) * sizeof(*l_write_buffer);
}


/**
 * XOR-encode an array with multiple reference points
 *
 *	parameters:
 *		read_buff	- the original array to be compressed
 *		write_buff	- the array to store the compressed data
 *		n_layers	- number of words in a line (i.e. row)
 *		n_ref		- number of reference points
 */
size_t encode_multiref(
		void *read_buffer, size_t read_buffer_size,
		void *write_buffer, size_t write_buffer_size,
		size_t n_layers, size_t n_ref)
{
	/*preprocessing*/
	uint32_t *l_read_buffer = (uint32_t *) read_buffer;
	uint32_t *l_write_buffer = (uint32_t *) write_buffer;

	/*setup the auxiliary variables*/
	size_t read_buffer_count = read_buffer_size / sizeof(*l_read_buffer);
	size_t rows = read_buffer_count / n_layers;
	size_t stride_lines = (rows + n_ref - 1) / n_ref;

	/*store the reference points*/
	size_t ref_idx = 0;
	for (; ref_idx < n_ref; ref_idx++)
	{
		memcpy(l_write_buffer + ref_idx * n_layers,
				l_read_buffer + stride_lines * ref_idx * n_layers,
				sizeof(*l_read_buffer) * n_layers);
	}

	/* init the remaining space*/
	memset(l_write_buffer + n_ref * n_layers,
			0,
			(rows - n_ref) * n_layers * sizeof(*l_write_buffer));

	/* init cursor*/
	uint32_t *cur = l_write_buffer + n_ref * n_layers;
	int bit_position = 0;

	/* updated on 7/2/2013:
	 * 		change the pointer to the word offset from the base
	 * Set the pointer to the first reference point*/
	dict[0] = (long)n_ref * n_layers;
	dict[1] = (long)0;
//
//	log_msg("d717 %s-%d: stride_lines = %ld, rows = %ld, \n",
//			__FILE__, __LINE__, stride_lines, rows);


	/*encode the data*/
	register size_t i, j;
	for (i = 1; i < rows; i++) /*we don't need to process the first line*/
	{
		for (j = 0; j < n_layers; j++)
		{
//
//			log_msg("d717 %s-%d: i = %d, l_read_buffer[%ld] = %f, \n",
//										__FILE__, __LINE__, i, i * n_layers + j,
//										*((float*)l_read_buffer + i * n_layers + j));

			uint32_t prevValue = l_read_buffer[(i - 1) * n_layers + j];
			uint32_t curValue = l_read_buffer[i * n_layers + j];

//			log_msg("d717 %s-%d: j = %d, stride_lines = %ld, \n",
//							__FILE__, __LINE__, j, stride_lines);

			packValue(prevValue, curValue, &cur, &bit_position);

//			//TODO
//			log_msg("d717 %s-%d: j = %d, stride_lines = %ld, \n",
//							__FILE__, __LINE__, j, stride_lines);

		}


		if (! (i % stride_lines)) /* Set the reference pointers except for ref0 */
		{
			int idx = i / stride_lines;
			dict[idx * 2] = (cur - l_write_buffer);
			dict[idx * 2 + 1] = (long)bit_position;
		}
	}

	/*print the compressed data*/
//    for (i = 0; i < 60; i++)
//    {
//    	printf("INFO %s-%d: l_write_buffer[%d] = %x \n", __FILE__, __LINE__, i, l_write_buffer[i]);
//    }

//	log_msg("dict[0] = %x, dict[1] = %d, *(uint32_t *)(l_write_buffer + dict[0]) = %x, \n",
//			dict[0], dict[1], *(uint32_t *)(l_write_buffer + dict[0]));
//	log_msg("dict[2] = %x, dict[3] = %d \n", dict[2], dict[3]);

	// so you are returning the size of the compressed data
	return (cur - l_write_buffer + 1) * sizeof(*l_write_buffer);
}

/**
 * The opposite of encode_xor()
 */
size_t decode_xor( void *read_buffer, size_t read_buffer_size,
		   void *write_buffer, size_t write_buffer_size,
		   size_t n_layers)
{
	uint32_t *l_read_buffer = (uint32_t *) read_buffer;
	uint32_t *l_write_buffer = (uint32_t *) write_buffer;

	size_t write_buffer_count = write_buffer_size / sizeof(*l_write_buffer);
	assert((write_buffer_count % n_layers)==0);

	/*simply copy over the base values, i.e. first row*/
	memcpy(l_write_buffer, l_read_buffer, sizeof(*l_read_buffer) * n_layers);

	
	size_t rows = write_buffer_size / sizeof(*l_write_buffer) / n_layers;
	uint32_t * cur = l_read_buffer + n_layers;

	int bit_position = 0;
	register size_t i, j;
	for(i = 1; i < rows; i++)
	{
		for(j = 0; j < n_layers; j++)
		{
		  uint32_t prevValue = l_write_buffer[(i - 1) * n_layers + j];
		  uint32_t curValue = unpackValue(prevValue, &cur, &bit_position);

		  l_write_buffer[i*n_layers + j] =  curValue;
		}
	}

	size_t decompressed_chunk_size = rows * n_layers * sizeof(*l_write_buffer);
	return decompressed_chunk_size;
}

/**
 * Decode the (partial) compressed file with multiple reference points
 *
 * Assume the decompressing data subset are aligned with lines
 * Note: will support partial lines in future
 *
 * Parameter:
 * 	pos_start		- the requested starting word for decompression
 * 	len				- number of words to be decompressed
 */
size_t decode_multiref( void *read_buffer, size_t read_buffer_size,
		   void *write_buffer, size_t write_buffer_size,
		   size_t n_layers, size_t n_ref,
		   size_t pos_start, size_t len)
{
	/*setup auxiliary variables*/
	uint32_t *l_read_buffer = (uint32_t *) read_buffer;
	uint32_t *l_write_buffer = (uint32_t *) write_buffer;
	size_t write_buffer_count = write_buffer_size / sizeof(*l_write_buffer);
	assert(0 == (write_buffer_count % n_layers));

	size_t rows = write_buffer_size / sizeof(*l_write_buffer) / n_layers;
	size_t stride_lines = (rows + n_ref - 1) / n_ref; /* #lines per stride */
	size_t stride_words = stride_lines * n_layers; /* #words per stride */
	size_t stride_idx_start = pos_start / stride_words; /* the i-th ref point to start from */
	size_t line_idx_start = stride_idx_start * stride_lines; /* starting line */
	size_t line_idx_end = (pos_start + len - 1) / n_layers; /* ending line */

	/*copy over the reference point*/
	memcpy(l_write_buffer,
			l_read_buffer + stride_idx_start * n_layers,
			sizeof(*l_write_buffer) * n_layers);

	/*load the base address*/
	uint32_t *cur = l_read_buffer + dict[stride_idx_start * 2];
	int bit_position = dict[stride_idx_start * 2 + 1];

//	log_msg("d78 %s-%d: *cur = %x, dict = %x, stride_idx_start = %d \n",
//			__FILE__, __LINE__, *cur, dict, stride_idx_start);
//	log_msg("d78 %s-%d: bit_position = %d \n", __FILE__, __LINE__, bit_position);

	/* decompress data */
	register size_t i, j;
	for(i = 1; i <= line_idx_end - line_idx_start; i++)
	{
		for(j = 0; j < n_layers; j++)
		{
		  uint32_t prevValue = l_write_buffer[(i-1)*n_layers + j];
		  uint32_t curValue = unpackValue(prevValue, &cur, &bit_position);

		  l_write_buffer[i*n_layers + j] =  curValue;
		}
	}

	size_t decompressed_chunk_size = (line_idx_end - line_idx_start + 1) * n_layers * sizeof(*l_write_buffer);
	return decompressed_chunk_size;
}

/**
 * Test single pair of floats
 */
int test_float_compress()
{
	printf("============================================================= \n");
	printf("===== Easiest case: compressed delta is within one word ===== \n");
	printf("============================================================= \n\n");

	float compressedData[10] = { 0 }; /*best practice: always init an array*/

	float prevValue = 16.000004f;
	float curValue = 16.000010f;

	uint32_t * cur = (uint32_t *) compressedData;
	int position = 0;

	packValue(*(uint32_t *) &prevValue, *(uint32_t *) &curValue, &cur, &position);

	/*DFZ: start uncompressing*/
	cur = (uint32_t *) compressedData;
	position = 0;
	uint32_t recoveredValue = unpackValue(*(uint32_t *) &prevValue, &cur, &position);

	if (DEBUG)
		printf("recoveredValue: %f\n", *(float *) &recoveredValue);

	assert(*(float *) &recoveredValue == curValue);
	printf("Success. \n\n");

	printf("===========================================================\n");
	printf("===== Testing 32 0's, i.e. prev and curr are the same =====\n");
	printf("===========================================================\n\n");

	memset(compressedData, 0, sizeof(float) * 10);

	prevValue = 16.000004f;
	curValue = prevValue;

	position = 0;
	cur = (uint32_t *) compressedData;
	if (DEBUG)
		printf("prevValue = %x, %f, \n", *(uint32_t *) &prevValue, prevValue);

	packValue(*(uint32_t *) &prevValue, *(uint32_t *) &curValue, &cur, &position);
	if (DEBUG) {
		printf("prevValue = %x, %f, \n", *(uint32_t *) &prevValue, prevValue);
	}

	position = 0;
	cur = (uint32_t *) compressedData;

	recoveredValue = unpackValue(*(uint32_t *) &prevValue, &cur, &position);
	if (DEBUG)
		printf("recoveredValue: %f\n", *(float *) &recoveredValue);

	assert(*(float *) &recoveredValue == curValue);
	printf("Success. \n\n");

	printf("================================================================= \n");
	printf("===== Testing the case: compressed delta is beyond one word ===== \n");
	printf("================================================================= \n\n");

	memset(compressedData, 0, sizeof(float) * 10);

	prevValue = 1.0f;
	curValue = 2.0f;
	if (DEBUG)
	{
		printf("prevValue = %x, %f, curValue = %x, %f \n",
				*(uint32_t *) &prevValue, prevValue, *(uint32_t *) &curValue,
				curValue);
	}

	position = 0;
	cur = (uint32_t *) compressedData;

	if (DEBUG)
	{
		printf("Line %d: (uint64_t) cur = %lx, \n", __LINE__,
				(long unsigned int)*(uint64_t *) cur);
	}

	packValue(*(uint32_t *) &prevValue, *(uint32_t *) &curValue, &cur, &position);

	position = 0;
	cur = (uint32_t *) compressedData;
	recoveredValue = unpackValue(*(uint32_t *) &prevValue, &cur, &position);

	if (DEBUG)
		printf("recoveredValue: %f\n", *(float *) &recoveredValue);

	assert(*(float *) &recoveredValue == curValue);
	printf("Success. \n\n");

	return 0;
}

int test_compress()
{
//#define NLAT 10 /*NLAT is not used for now, except for allocating memory for arrays*/
//#define NLON 10
//#define NLAYER 10

	int rank = 0;
	float data[NLAT * NLON * NLAYER];
	int lat, lon, layer;
	for (lat = 0; lat < NLAT; lat++)
		for (lon = 0; lon < NLON; lon++)
			for (layer = 0; layer < NLAYER; layer++)
			{
				data[lat * NLON * NLAYER + lon * NLAYER + layer] = rank * 1000
						+ lat * 1 + lon * 0.1 + layer * 0.001;
			}

	/*NOTE: in theory, it is possible to have an even larger required space
	 * in compressedData. Consider doubling the length.*/
	float compressedData[NLAT * NLON * NLAYER] = {0}; /*Best Practice: always init the array*/
	float unCompressedData[NLAT * NLON * NLAYER] = {0}; /*Best Practice: always init the array*/

//#define NREF 5
	size_t ret = encode_multiref(data, NLON * NLAYER * sizeof(float),
			compressedData,	NLON * NLAYER * sizeof(float),
			NLAYER, NREF);

	/*DFZ: modify the last two parameters to change decompression settings*/
	ret = decode_multiref(compressedData, NLON * NLAYER * sizeof(float),
			unCompressedData, NLON * NLAYER * sizeof(float),
			NLAYER, NREF,
			39, NLON * NLAYER / 2); /*start position, length*/

	if (DEBUG)
		printf("%d: ret = %ld \n", __LINE__, (long int)ret);

//#define MAXLAT 1

	printf("\n");
	for (lat = 0; lat < MAXLAT; lat++)
		for (lon = 0; lon < NLON; lon++)
		{
			for (layer = 0; layer < NLAYER; layer++)
			{
				printf(" %f", data[lat * NLON * NLAYER + lon * NLAYER + layer]);
			}
			printf("\n");
		}

	printf("\n");
	for (lat = 0; lat < MAXLAT; lat++)
		for (lon = 0; lon < NLON; lon++)
		{
			for (layer = 0; layer < NLAYER; layer++) {
				printf(" %f",
						unCompressedData[lat * NLON * NLAYER + lon * NLAYER	+ layer]);
			}
			printf("\n");
		}

	return 0;
}
//
//int main(int argc, char * argv[])
//{
//  test1();
//  test();
//	uint32_t* tmp_ary[4] = {0};
//	printf("&(tmp_ary[3]) - tmp_ary = %d, tmp_ary = %x, &(tmp_ary[3]) = %x \n",
//			&(tmp_ary[3]) - tmp_ary, tmp_ary, &(tmp_ary[3]));
//}

