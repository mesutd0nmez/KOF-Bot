#include "pch.h"
#include "Service.h"
#include "Bot.h"

Service::Service()
{
}

Service::~Service()
{
    CloseSocket();
}

void Service::Initialize()
{
#ifdef VMPROTECT
    VMProtectBeginUltra("Service::Initialize");
#endif

    m_Cryption->SetEncryptionKey(skCryptDec("fm3eJbEWuc556JcPQF6eDtabdm38KuRMEjyPwfEazsAcJwjYhwznShMEtPa4DH4f"));

    GenerateSeed((1881 * 2023) / 2009 << 16);
    m_Cryption->SetInitialVector(std::to_string(GetSeed()));

#ifdef _DEBUG
    Connect(skCryptDec("127.0.0.1"), 15000);
#else
    Connect(skCryptDec("54.37.199.67"), 15000);
#endif 

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::OnConnect()
{
    OnConnected();
}

void Service::OnError(int32_t iErrorCode)
{
#ifdef DEBUG_LOG
    Print("Connection error: %d", iErrorCode);
#endif

#ifdef ENABLE_SERVER_CONNECTION_LOST_CHECK
    exit(0);
#endif
}

void Service::OnClose()
{
#ifdef ENABLE_SERVER_CONNECTION_LOST_CHECK
    exit(0);
#endif
}

void Service::HandlePacket(Packet& pkt)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::HandlePacket");
#endif

    uint8_t iHeader;

    pkt >> iHeader;

    switch (iHeader)
    {
        case PacketHeader::READY:
        {
            pkt >> m_iId;

            GenerateSeed(m_iId + GetCurrentProcessId());
            m_Cryption->SetInitialVector(std::to_string(GetSeed()));

            OnReady();

        }
        break;

        case PacketHeader::LOGIN:
        {
            pkt.DByte();

            uint8_t iType, iStatus;
            pkt >> iType >> iStatus;

            switch (iStatus)
            {
                case 0: //Error - Authentication Failed
                {
                    std::string szAuthenticationMessage;

                    pkt >> szAuthenticationMessage;

                    OnAuthenticationMessage(true, szAuthenticationMessage);
                }
                break;

                case 1: //Success - Socket authenticated
                case 2: //Success - Socket authenticated but client update needed
                {
                    OnAuthenticationMessage(false, "");

                    switch (iType)
                    {
                        case LoginType::GENERIC:
                        case LoginType::TOKEN:
                        {
                            std::string szToken;
                            uint32_t iSubscriptionEndAt;

                            pkt >> szToken >> iSubscriptionEndAt;

                            OnSaveToken(szToken, iSubscriptionEndAt);
                            OnAuthenticated(iStatus);
                        }
                        break;
                    }
                }
                break;
            }
        }
        break;

        case PacketHeader::CONFIGURATION:
        {
            uint8_t iType;

            pkt.DByte();
            pkt >> iType;

            switch (iType)
            {
            case ConfigurationRequestType::LOAD:
            {
                int iBufferLength;

                pkt >> iBufferLength;

                std::vector<uint8_t> vecBuffer(iBufferLength);

                if (iBufferLength > 0)
                {
                    pkt.read(&vecBuffer[0], iBufferLength);
                }         

                std::string szData(reinterpret_cast<const char*>(vecBuffer.data()), vecBuffer.size());
                OnConfigurationLoaded(szData);
            }
            break;
            }
        }
        break;

        case PacketHeader::POINTER:
        {
            int iBufferLength;

            pkt >> iBufferLength;

            std::vector<uint8_t> vecBuffer(iBufferLength);

            if (iBufferLength > 0)
            {
                pkt.read(&vecBuffer[0], iBufferLength);
            }       

            std::string szData((char*)vecBuffer.data(), vecBuffer.size());
            OnLoaded(szData);
        }
        break;

        case PacketHeader::PING:
        {
            pkt.DByte();

            uint32_t iSubscriptionEndAt;
            pkt >> iSubscriptionEndAt;

            OnPong(iSubscriptionEndAt);
        }
        break;

        case PacketHeader::CAPTCHA:
        {
            uint8_t iStatus;

            pkt.DByte();
            pkt >> iStatus;

            std::string szResult;
            pkt.readString(szResult);

            OnCaptchaResponse(iStatus == 1 ? true : false, szResult);
        }
        break;

        case PacketHeader::INJECTION:
        {
            int32_t iAdapterBufferLength;

            pkt >> iAdapterBufferLength;

            std::vector<uint8_t> vecAdapterBuffer(iAdapterBufferLength);

            if (iAdapterBufferLength > 0)
            {
                pkt.read(&vecAdapterBuffer[0], iAdapterBufferLength);
            }      

            OnInjection(vecAdapterBuffer);
        }
        break;

        case PacketHeader::UPDATE:
        {
            int32_t iUpdateBufferLength, iUpdaterBufferLength;

            pkt >> iUpdateBufferLength;

            std::vector<uint8_t> vecUpdateBuffer(iUpdateBufferLength);

            if (iUpdateBufferLength > 0)
            {
                pkt.read(&vecUpdateBuffer[0], iUpdateBufferLength);
            }          

            pkt >> iUpdaterBufferLength;

            std::vector<uint8_t> vecUpdaterBuffer(iUpdaterBufferLength);

            if (iUpdaterBufferLength > 0)
            {
                pkt.read(&vecUpdaterBuffer[0], iUpdaterBufferLength);
            }        

            std::ofstream updateFile(skCryptDec("Update.zip"), std::ios::binary);
            std::ofstream updaterFile(skCryptDec("Updater.exe"), std::ios::binary);

            if (!updateFile.is_open() || !updaterFile.is_open())
            {
                OnUpdateDownloaded(false);
                return;
            }

            updateFile.write(reinterpret_cast<const char*>(vecUpdateBuffer.data()), vecUpdateBuffer.size());
            updateFile.close();

            updaterFile.write(reinterpret_cast<const char*>(vecUpdaterBuffer.data()), vecUpdaterBuffer.size());
            updaterFile.close();

            OnUpdateDownloaded(true);
        }
        break;

        case PacketHeader::ROUTE:
        {
            int32_t iFileCount;

            pkt >> iFileCount;

            for (size_t i = 0; i < (size_t)iFileCount; i++)
            {
                int32_t iBufferLength;

                pkt >> iBufferLength;

                std::vector<uint8_t> vecBuffer(iBufferLength);

                if (iBufferLength > 0)
                {
                    pkt.read(&vecBuffer[0], iBufferLength);
                }           

                OnRouteLoaded(vecBuffer);
            }
        }
        break;
    }

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::SendReady(std::string szProcessFileName, uint32_t iCRC, HardwareInformation* pHardwareInformation)
{
    Packet pkt = Packet(PacketHeader::READY);

    pkt.DByte();
    pkt
        << uint32_t(GetCurrentProcessId()) 
        << szProcessFileName
        << uint32_t(iCRC) 
        << to_string(pHardwareInformation->System.Name.c_str())
        << to_string(pHardwareInformation->Registry.ComputerHardwareId.c_str())
        << to_string(pHardwareInformation->System.OSSerialNumber.c_str());

    std::string szGPUs;
    for (size_t i = 0; i < pHardwareInformation->GPU.size(); i++)
    {
        if (i == 0)
            szGPUs += to_string(pHardwareInformation->GPU.at(i).Name.c_str());
        else
            szGPUs += "||" + to_string(pHardwareInformation->GPU.at(i).Name.c_str());
    }

    pkt << szGPUs;

    Send(pkt);
}

void Service::SendLogin(std::string szEmail, std::string szPassword)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::SendLogin1");
#endif

    Packet pkt = Packet(PacketHeader::LOGIN);

    pkt.DByte();
    pkt
        << uint8_t(LoginType::GENERIC)
        << szEmail.c_str()
        << szPassword.c_str();

    Send(pkt);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::SendLogin(std::string szToken)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::SendLogin2");
#endif

    Packet pkt = Packet(PacketHeader::LOGIN);

    pkt.DByte();
    pkt
        << uint8_t(LoginType::TOKEN)
        << szToken.c_str();

    Send(pkt);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::SendPointerRequest(PlatformType ePlatformType)
{
    Packet pkt = Packet(PacketHeader::POINTER);

    pkt.DByte();
    pkt 
        << uint8_t(ePlatformType);

    Send(pkt);
}

void Service::SendLoadUserConfiguration(uint8_t iServerId, std::string szCharacterName, PlatformType ePlatformType)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    pkt.DByte();
    pkt 
        << uint8_t(ConfigurationRequestType::LOAD) 
        << uint8_t(ePlatformType)
        << uint8_t(iServerId)
        << szCharacterName;

    Send(pkt);
}

void Service::SendSaveUserConfiguration(uint8_t iServerId, std::string szCharacterName, std::string szConfigurationData, PlatformType ePlatformType)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    pkt.DByte();
    pkt
        << uint8_t(ConfigurationRequestType::SAVE)
        << uint8_t(ePlatformType)
        << uint8_t(iServerId)
        << szCharacterName
        << uint32_t(szConfigurationData.size());

    pkt.append(szConfigurationData);

    Send(pkt, true);
}

void Service::SendInjectionRequest()
{
    Packet pkt = Packet(PacketHeader::INJECTION);

    pkt.DByte();
    pkt
        << uint8_t(PlatformType::USKO);

    Send(pkt);
}

void Service::SendPong(std::string szCharacterName, float fX, float fY, uint8_t iZoneID)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::SendPong");
#endif

    Packet pkt = Packet(PacketHeader::PING);

    pkt.DByte();
    pkt << szCharacterName
        << float(fX)
        << float(fY)
        << uint8_t(iZoneID);

    Send(pkt);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::SendCaptcha(std::vector<uint8_t> vecImageBuffer)
{
    Packet pkt = Packet(PacketHeader::CAPTCHA);

    pkt.DByte();
    pkt.append(vecImageBuffer.data(), vecImageBuffer.size());

    Send(pkt, true);
}

void Service::SendUpdate()
{
    Packet pkt = Packet(PacketHeader::UPDATE);

    Send(pkt);
}

void Service::SendRouteLoadRequest()
{
    Packet pkt = Packet(PacketHeader::ROUTE);

    pkt.DByte();
    pkt << uint8_t(0);

    Send(pkt);
}

void Service::SendRouteSaveRequest(std::string szName, JSON jData)
{
    Packet pkt = Packet(PacketHeader::ROUTE);

    std::string szData = jData.dump();

    pkt.DByte();
    pkt
        << uint8_t(1)
        << szName
        << uint32_t(szData.size());

    pkt.append(szData);

    Send(pkt, true);
}

void Service::SendRouteDeleteRequest(std::string szName)
{
    Packet pkt = Packet(PacketHeader::ROUTE);

    pkt.DByte();
    pkt 
        << uint8_t(2) 
        << szName;

    Send(pkt);
}

void Service::SendReport(uint32_t iCode, std::string szPayload)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::SendReport");
#endif

    Packet pkt = Packet(PacketHeader::REPORT);

    pkt.DByte();
    pkt
        << uint32_t(iCode)
        << szPayload;

    Send(pkt);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::SendScreenshot(std::vector<uint8_t> vecImageBuffer)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::SendScreenshot");
#endif

    Packet pkt = Packet(PacketHeader::SCREENSHOT);

    pkt.DByte();
    pkt.append(vecImageBuffer.data(), vecImageBuffer.size());

    Send(pkt, true);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

void Service::SendVital(uint32_t iCode, std::string szPayload)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("Service::SendVital");
#endif

    Packet pkt = Packet(PacketHeader::VITAL);

    pkt.DByte();
    pkt
        << uint32_t(iCode)
        << szPayload;

    Send(pkt);

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}