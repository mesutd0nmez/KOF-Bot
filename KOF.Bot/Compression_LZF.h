#pragma once

namespace Compression
{
	namespace LZF
	{
		std::vector<uint8_t> Compression(const std::vector<uint8_t>& in_data);
		std::vector<uint8_t> Decompression(const std::vector<uint8_t>& in_data, uint32_t original_len);
	}
}
