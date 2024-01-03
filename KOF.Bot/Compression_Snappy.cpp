#include "pch.h"
#include "Compression.h"

namespace Compression
{
	namespace Snappy
	{
		std::vector<uint8_t> Compression(const std::vector<uint8_t>& in_data)
		{
#ifdef VMPROTECT
			VMProtectBeginMutation("Snappy::Compression");
#endif
			size_t out_len = snappy_max_compressed_length(in_data.size());
			std::vector<uint8_t> out_data(out_len);

			try
			{
				snappy_status pStatus = snappy_compress((char*)in_data.data(), in_data.size(), (char*)out_data.data(), &out_len);

				if (pStatus == SNAPPY_OK)
				{
					out_data.resize(out_len);
				}
				else
				{
#ifdef DEBUG_LOG
					Print("Compression Failed: %d - original_len(%d) - out_len(%d)", (int)pStatus, in_data.size(), out_len);
#endif

					out_data.resize(0);
				}
			}
			catch (const std::exception& e)
			{
#ifdef DEBUG_LOG
				Print("%s", e.what());
#else
				UNREFERENCED_PARAMETER(e);
#endif

				out_data.resize(0);
			}

			return out_data;

#ifdef VMPROTECT
			VMProtectEnd();
#endif
		}

		std::vector<uint8_t> Decompression(const std::vector<uint8_t>& in_data, uint32_t original_len)
		{
#ifdef VMPROTECT
			VMProtectBeginMutation("Snappy::Decompression");
#endif

			size_t out_len = original_len;
			std::vector<uint8_t> out_data(out_len);

			try
			{
				snappy_status pStatus = snappy_uncompress((char*)in_data.data(), in_data.size(), (char*)out_data.data(), &out_len);

				if (pStatus == SNAPPY_OK 
					&& original_len == out_len)
				{
					out_data.resize(out_len);
				}
				else
				{
#ifdef DEBUG_LOG
					Print("Decompression Failed: %d - original_len(%d) - out_len(%d)", (int)pStatus, original_len, out_len);
#endif

					out_data.resize(0);
				}
				
			}
			catch (const std::exception& e)
			{
#ifdef DEBUG_LOG
				Print("%s", e.what());
#else
				UNREFERENCED_PARAMETER(e);
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
