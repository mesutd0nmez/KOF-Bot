#pragma once

#include "Compression_LZF.h"
#include "Compression_Snappy.h"

namespace Compression
{
	static const size_t MinBytes = 512;

	using LZF::Compression;
	using LZF::Decompression;

	std::vector<uint8_t> Compress(const std::vector<uint8_t>& in_data);
	std::vector<uint8_t> Decompress(const std::vector<uint8_t>& in_data, uint32_t original_len);
}
