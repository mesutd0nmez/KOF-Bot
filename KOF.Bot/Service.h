#pragma once

#include "Enum.h"
#include "Socket.h"
#include "Ini.h"
#include "HardwareInformation.h"

class Service : public Socket
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
	virtual void OnUpdate() = 0;
	virtual void OnUpdateDownloaded(bool bStatus) = 0;
	virtual void OnConnected() = 0;
	virtual void OnLoaded() = 0;
	virtual void OnConfigurationLoaded() = 0;
	virtual void OnCaptchaResponse(bool bStatus, std::string szResult) = 0;

protected:
	void SendLogin(std::string szToken);

public:
	void SendReady(uint32_t iCRC);
	void SendPointerRequest();

private:
	void HandlePacket(Packet& pkt);
	void OnConnect();
	void OnError(int32_t iErrorCode);
	void OnClose(int32_t iErrorCode);

public:
	void SendInjectionRequest(uint32_t iProcessId);
	void SendInjectionReport(bool bStarted, uint32_t iProcessId);

protected:
	void SendPong(std::string szName, float fX, float fY, uint8_t iZoneID);

protected:
	Ini* m_iniPointer;
	Ini* m_iniAppConfiguration;
	Ini* m_iniUserConfiguration;
	bool m_bUserSaveRequested;
	std::string m_szToken;

public:
	PlatformType m_ePlatformType;

private:
	bool m_isServiceClosed;

public:
	bool IsServiceClosed() { return m_isServiceClosed; };

protected:
	HardwareInformation m_hardwareInfo;

public:
	void SendCaptcha(std::vector<uint8_t> vecImageBuffer);
	void SendLogin(std::string szEmail, std::string szPassword);

public:
	bool m_isAuthenticationFailed;
	std::string m_szAuthenticationFailedMessage;

public: 
	void SendUpdate();

public:
	bool m_bOnReady;

public:
	typedef std::map<std::string, std::vector<Route>> RouteList;
	typedef std::map<uint8_t, RouteList> RouteData;

public:
	bool GetRouteList(uint8_t iMapIndex, RouteList& pRouteList);
	void SaveRoute(std::string szRouteName, uint8_t iMapIndex, std::vector<Route> vecRoute);
	void DeleteRoute(std::string szRouteName, uint8_t iMapIndex);

public:
	void SendRouteLoadRequest();
	void SendRouteSaveRequest(std::string szName, JSON jData);
	void SendRouteDeleteRequest(std::string szName);

private:
	RouteData m_mapRouteList;

public:
	bool m_bAutoLogin;
	bool m_bAutoStart;

	std::string m_szID;
	std::string m_szPassword;

	int m_iServerID;
	int m_iChannelID;
	int m_iSlotID;

	uint32_t m_iSubscriptionEndAt;

	std::string m_szAnyOTPID;
	std::string m_szAnyOTPPassword;
};

