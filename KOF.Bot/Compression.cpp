#include "pch.h"
#include "Compression.h"

std::vector<uint8_t> Compression::Compress(const std::vector<uint8_t>& in_data)
{
	return Compression(in_data);
}

std::vector<uint8_t> Compression::Decompress(const std::vector<uint8_t>& in_data, uint32_t original_len)
{
	return Decompression(in_data, original_len);
}
