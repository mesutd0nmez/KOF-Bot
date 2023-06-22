#pragma once

#include "Enum.h"
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
	virtual void OnAuthenticated() = 0;
	virtual void OnLoaded() = 0;
	virtual void OnConfigurationLoaded() = 0;

protected:
	void SendLogin(std::string szToken);

private:
	void SendReady();
	void SendPointerRequest();

private:
	void HandlePacket(Packet& pkt);
	void OnConnect();
	void OnError(int32_t iErrorCode);
	void OnClose(int32_t iErrorCode);

protected:
	void SendInjectionRequest(uint32_t iProcessId);

protected:
	Ini* m_iniPointer;
	Ini* m_iniAppConfiguration;
	Ini* m_iniUserConfiguration;
	std::string m_szToken;

public:
	PlatformType m_ePlatformType;
	int32_t m_iSelectedAccount;

private:
	bool m_isServiceClosed;

public:
	bool IsServiceClosed() { return m_isServiceClosed; };

protected:
	uint32_t m_iAccountTypeFlag;

};

