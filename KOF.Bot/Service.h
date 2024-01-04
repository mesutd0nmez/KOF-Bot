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

	public:
		void Initialize();
		void CloseSocket() { m_tcpSocket.Close(); };

	private:
		void OnConnect();
		void OnError(int32_t iErrorCode);
		void OnClose();

		virtual void OnReady() = 0;
		virtual void OnConnected() = 0;
		virtual void OnPong(uint32_t iSubscriptionEndAt, int32_t iCredit) = 0;
		virtual void OnAuthenticated(uint8_t iStatus) = 0;
		virtual void OnSaveToken(std::string szToken, uint32_t m_iSubscriptionEndAt, int32_t iCredit) = 0;
		virtual void OnAuthenticationMessage(bool bStatus, std::string szMessage) = 0;
		virtual void OnUpdate() = 0;
		virtual void OnUpdateDownloaded(bool bStatus) = 0;
		virtual void OnLoaded(std::string szPointerData) = 0;
		virtual void OnInjection(std::vector<uint8_t> vecBuffer) = 0;
		virtual void OnConfigurationLoaded(std::string szConfiguration) = 0;
		virtual void OnRouteLoaded(std::vector<uint8_t> vecBuffer) = 0;
		virtual void OnCaptchaResponse(bool bStatus, std::string szResult) = 0;
		virtual void OnPurchase(std::string szPurchaseUrl) = 0;

	private:
		void HandlePacket(Packet& pkt);

	protected:
		void SendSaveUserConfiguration(uint8_t iServerId, std::string szCharacterName, std::string szConfigurationData, PlatformType ePlatformType);
		void SendReady(std::string szProcessFileName, uint32_t iCRC, HardwareInformation* pHardwareInformation);
		void SendPointerRequest(PlatformType ePlatformType);
		void SendInjectionRequest();
		void SendPong(std::string szName, float fX, float fY, uint8_t iZoneID);
		void SendUpdate();
		void SendRouteLoadRequest();
		void SendRouteDeleteRequest(std::string szName);
		void SendLogin(std::string szToken);

	public:	
		void SendLogin(std::string szEmail, std::string szPassword);
		void SendCaptcha(std::vector<uint8_t> vecImageBuffer);
		void SendLoadUserConfiguration(uint8_t iServerId, std::string szCharacterName, PlatformType ePlatformType);
		void SendRouteSaveRequest(std::string szName, JSON jData);
		void SendReport(uint32_t iCode, std::string szPayload = "");
		void SendScreenshot(std::vector<uint8_t> vecImageBuffer);
		void SendVital(uint32_t iCode, std::string szPayload = "");
		void SendPurchase(uint8_t iType, int32_t iCredit, uint32_t iDay);
};

