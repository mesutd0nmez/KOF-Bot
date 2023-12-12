#pragma once

#include "ByteBuffer.h"
#include "PacketHeader.h"
#include <cstdint>
#include <string>

class Packet : public ByteBuffer
{
public:
	inline Packet() : ByteBuffer()
	{
	}

	inline Packet(uint8_t opcode) : ByteBuffer(1024000)
	{
		append(&opcode, 1);
	}

	inline Packet(uint8_t opcode, size_t res) : ByteBuffer(res)
	{
		append(&opcode, 1);
	}

	inline Packet(const Packet& packet) : ByteBuffer(packet)
	{
	}

	inline uint8_t GetOpcode() const
	{
		return size() == 0 ? 0 : _storage[0];
	}

	inline std::string toHex()
	{
		return convertToHex();
	}

	//! Clear packet and set opcode all in one mighty blow
	inline void Initialize(uint8_t opcode)
	{
		clear();
		append(&opcode, 1);
	}
};