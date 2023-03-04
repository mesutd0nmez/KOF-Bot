#pragma once

#include "Packet.h"
#include "Cryption.h"
#include "Compression.h"
#include "TcpSocket.h"

class Socket
{
public:
	Socket();
	~Socket();

	void Connect(std::string szIP, uint16_t iPort);
	void Close();
	bool IsClosed() { return m_tcpSocket.isClosed; };
	bool IsError() { return m_tcpSocket.isError; };
	bool IsConnected() { return m_tcpSocket.isConnected; };

	void GenerateSeed(uint32_t x);
	uint32_t GetSeed() { return m_iSeed; };

	Cryption* GetCryption() { return m_Cryption; };

public:
	void Send(Packet& pkt, bool bCompress = false);

private:
	void ProcessPacket(uint8_t* iStream, size_t iStreamLength);

private:
	virtual void HandlePacket(Packet& pkt) = 0;
	virtual void OnError(int32_t iErrorCode) = 0;
	virtual void OnConnect() = 0;
	virtual void OnClose(int32_t iErrorCode) = 0;

private:
	uint32_t m_iSeed;
	Compression* m_Compression;

protected:
	int32_t m_iId;
	TCPSocket m_tcpSocket;
	Cryption* m_Cryption;
};

