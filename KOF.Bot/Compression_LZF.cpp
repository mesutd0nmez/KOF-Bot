#include "pch.h"
#include "Compression.h"
#include "lzf.h"

namespace Compression
{
	namespace LZF
	{
		std::vector<uint8_t> Compression(const std::vector<uint8_t>& in_data)
		{
			uint32_t max_out_len = in_data.size() + LZF_MARGIN;
			std::vector<uint8_t> out_data(max_out_len);

			uint32_t out_len = lzf_compress(in_data.data(), in_data.size(), out_data.data(), max_out_len);
			out_data.resize(out_len);

			return out_data;
		}

		std::vector<uint8_t> Decompression(const std::vector<uint8_t>& in_data, uint32_t original_len)
		{
#ifdef VMPROTECT
			VMProtectBeginMutation("LZF::Decompression");
#endif

			std::vector<uint8_t> out_data(original_len);

			uint32_t out_len = lzf_decompress(in_data.data(), in_data.size(), out_data.data(), original_len);

			if (original_len == out_len)
				out_data.resize(out_len);
			else
			{
#ifdef DEBUG_LOG
				Print("Decompression original_len(%d) - out_len(%d)", original_len, out_len);
#endif
				out_data.resize(0);
			}

			return out_data;
#ifdef VMPROTECT
			VMProtectEnd();
#endif
		}
	}
}
