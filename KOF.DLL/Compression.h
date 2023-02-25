#pragma once

#include <stdio.h>
#include <snappy-c.h>

class Compression
{
public:

	bool Compress(const char* inputData, size_t inputDataLength, char* compressedData, size_t* compressedDataLength)
	{
		snappy_status pStatus = snappy_compress(inputData, inputDataLength, compressedData, compressedDataLength);

		if (pStatus != SNAPPY_OK)
		{
#ifdef DEBUG
			printf("Snappy compression failed: %d\n", pStatus);
#endif
			return false;
		}

		return true;
	}

	bool UnCompress(const char * compressedData, size_t compressedDataLength, char* outputData, size_t *outputDataLength)
	{
		snappy_status pStatus = snappy_uncompress(compressedData, compressedDataLength, outputData, outputDataLength);

		if (pStatus != SNAPPY_OK)
		{
#ifdef DEBUG
			printf("Snappy decompression failed: %d\n", pStatus);
#endif
			return false;
		}

		return true;
	}
};

