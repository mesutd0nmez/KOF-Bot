#pragma once

#include "Socket.h"
#include "Ini.h"

class Service : Socket
{
public:
	Service();
	virtual ~Service();

	void Clear();

public:
	void Initialize();
	void CloseSocket() { m_tcpSocket.Close(); };

	 void SendLoadUserConfiguration(uint8_t iServerId, std::string szCharacterName) ;
	 void SendSaveUserConfiguration(uint8_t iServerId, std::string szCharacterName) ;

private:
	virtual void OnReady() = 0;
	virtual void OnPong() = 0;
	virtual void OnAuthenticated() = 0;
	virtual void OnLoaded() = 0;
	virtual void OnConfigurationLoaded() = 0;

private:
	void SendReady();
	void SendLogin(std::string szToken);
	void SendPointerRequest();
	void SendPong();

private:
	void HandlePacket(Packet& pkt);
	void OnConnect();
	void OnError(int32_t iErrorCode);
	void OnClose(int32_t iErrorCode);

protected:
	Ini* m_iniPointer;
	Ini* m_iniUserConfiguration;

private:
	std::string m_szToken;
	Ini* m_iniConfiguration;

};

