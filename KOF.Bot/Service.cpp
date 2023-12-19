#include "pch.h"
#include "Service.h"
#include "HardwareInformation.h"
#include "Bot.h"

Service::Service()
{
    Clear();
}

Service::~Service()
{
    Clear();
    CloseSocket();
}

void Service::Clear()
{
    m_szToken.clear();

    m_iniPointer = nullptr;
    m_iniUserConfiguration = nullptr;
    m_iniAppConfiguration = nullptr;

    m_ePlatformType = PlatformType::USKO;

    m_isServiceClosed = false;

    m_isAuthenticationFailed = false;
    m_szAuthenticationFailedMessage = "";

    m_bAutoLogin = false;
    m_bAutoStart = false;

    m_szID = "";
    m_szPassword = "";

    m_iServerID = 0;
    m_iChannelID = 0;
    m_iSlotID = 1;

    m_iSubscriptionEndAt = 0;

    m_szAnyOTPID.clear();
    m_szAnyOTPPassword.clear();
}

void Service::Initialize()
{
    std::string szIniPath = skCryptDec(".\\Config.ini");

    m_iniAppConfiguration = new Ini();
    m_iniAppConfiguration->Load(szIniPath.c_str());

    m_szToken = m_iniAppConfiguration->GetString(skCryptDec("Internal"), skCryptDec("Token"), m_szToken.c_str());
    m_bAutoLogin = m_iniAppConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Login"), m_bAutoLogin);
    m_bAutoStart = m_iniAppConfiguration->GetBool(skCryptDec("Automation"), skCryptDec("Start"), m_bAutoStart);

    m_szID = m_iniAppConfiguration->GetString(skCryptDec("AutoLogin"), skCryptDec("ID"), m_szID.c_str());
    m_szPassword = m_iniAppConfiguration->GetString(skCryptDec("AutoLogin"), skCryptDec("Password"), m_szPassword.c_str());

    m_iServerID = m_iniAppConfiguration->GetInt(skCryptDec("AutoLogin"), skCryptDec("Server"), m_iServerID);
    m_iChannelID = m_iniAppConfiguration->GetInt(skCryptDec("AutoLogin"), skCryptDec("Channel"), m_iChannelID);   
    m_iSlotID = m_iniAppConfiguration->GetInt(skCryptDec("AutoLogin"), skCryptDec("Slot"), m_iSlotID);

    m_szAnyOTPID = m_iniAppConfiguration->GetString(skCryptDec("AnyOTP"), skCryptDec("ID"), m_szAnyOTPID.c_str());
    m_szAnyOTPPassword = m_iniAppConfiguration->GetString(skCryptDec("AnyOTP"), skCryptDec("Password"), m_szAnyOTPPassword.c_str());

#ifdef VMPROTECT
    VMProtectBeginUltra("ServiceInitializeEncryptionKey");
#endif

    m_Cryption->SetEncryptionKey(skCryptDec("Dm9X2UPop7mghLqibu4aMgT7feXQmZkrrLwoWQMZKTK3dvoPKR2CCmUSJgHBxKn8"));

#ifdef VMPROTECT
    VMProtectEnd();
#endif

    GenerateSeed((1881 * 2023) / 2009 << 16);
    m_Cryption->SetInitialVector(std::to_string(GetSeed()));

#ifdef DEBUG
    Connect(skCryptDec("127.0.0.1"), 8888);
#else
    Connect(skCryptDec("51.195.101.149"), 8888);
#endif 
}

void Service::OnConnect()
{
#ifdef DEBUG
    printf("Connected to socket successfully\n");
#endif

    OnConnected();
}

void Service::OnError(int32_t iErrorCode)
{
#ifdef DEBUG
    printf("Connection error: %d\n", iErrorCode);
#endif

    m_isServiceClosed = true;
}

void Service::OnClose(int32_t iErrorCode)
{
#ifdef DEBUG
    printf("Connection closed: %d\n", iErrorCode);
#endif

    m_isServiceClosed = true;
}

void Service::HandlePacket(Packet& pkt)
{
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
                    m_isAuthenticationFailed = true;
                    m_szAuthenticationFailedMessage = "";

                    pkt >> m_szAuthenticationFailedMessage;

        #ifdef DEBUG
                    printf("Socket authentication failed: %s\n", m_szAuthenticationFailedMessage.c_str());
        #endif
                }
                break;

                case 1: //Success - Socket authenticated
                case 2: //Success - Socket authenticated but client update needed
                {
                    switch (iType)
                    {
                        case LoginType::GENERIC:
                        case LoginType::TOKEN:
                        {
                            std::string szToken;

                            pkt >> szToken >> m_iSubscriptionEndAt;

                            m_szToken = m_iniAppConfiguration->SetString(skCryptDec("Internal"), skCryptDec("Token"), szToken.c_str());
                        }
                        break;
                    }

                    m_isAuthenticationFailed = false;
                    m_szAuthenticationFailedMessage = "";

        #ifdef DEBUG
                    printf("Socket authenticated\n");
        #endif
                    switch (iStatus)
                    {
                        case 1:
                        {
                            SendPointerRequest();
                            OnAuthenticated();
                        }
                        break;

                        case 2:
                        {
            #ifdef DEBUG
                            SendPointerRequest();
                            OnAuthenticated();
            #else
                            OnUpdate();
            #endif
                        }
                        break;
                    }
                    break;
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

                m_iniUserConfiguration = new Ini();

                if (iBufferLength > 0)
                {
                    std::vector<uint8_t> vecBuffer(iBufferLength);
                    pkt.read(&vecBuffer[0], iBufferLength);

                    std::string szData(reinterpret_cast<const char*>(vecBuffer.data()), vecBuffer.size());

                    m_iniUserConfiguration->Load(szData);
                }         

                OnConfigurationLoaded();
            }
            break;
            }
        }
        break;

        case PacketHeader::POINTER:
        {
            int iBufferLength;

            pkt >> iBufferLength;

            m_iniPointer = new Ini();

            if (iBufferLength > 0)
            {
                std::vector<uint8_t> vecBuffer(iBufferLength);
                pkt.read(&vecBuffer[0], iBufferLength);

                std::string szData((char*)vecBuffer.data(), vecBuffer.size());

                m_iniPointer->Load(szData);
            }

            OnLoaded();
        }
        break;

        case PacketHeader::PING:
        {
            pkt.DByte();
            pkt >> m_iSubscriptionEndAt;

            OnPong();
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
            int32_t iProcesssId, iBufferLength;

            pkt >> iProcesssId >> iBufferLength;

            std::vector<uint8_t> vecBuffer(iBufferLength);
            pkt.read(&vecBuffer[0], iBufferLength);

            if (iBufferLength > 0)
            {
                //Injection(iProcesssId, "C:\\Users\\Administrator\\Documents\\GitHub\\KOF-Bot\\Debug\\Adapter.dll");

                char tempPath[MAX_PATH];
                GetTempPathA(MAX_PATH, tempPath);

                char tempDllFileName[MAX_PATH];
                GetTempFileNameA(tempPath, "", 0, tempDllFileName);

                std::ofstream dll(tempDllFileName, std::ios::binary);

                if (dll.is_open())
                {
                    dll.write(reinterpret_cast<const char*>(vecBuffer.data()), vecBuffer.size());
                    dll.close();

                    Injection(iProcesssId, tempDllFileName);

                    SendInjectionReport(true, iProcesssId);
                }
                else
                {
                    SendInjectionReport(false, iProcesssId);
                }
            }
            else
            {
                SendInjectionReport(false, iProcesssId);
            }
        }
        break;

        case PacketHeader::UPDATE:
        {
            int32_t iBufferLength;

            pkt >> iBufferLength;

            if (iBufferLength > 0)
            {
                std::vector<uint8_t> vecBuffer(iBufferLength);
                pkt.read(&vecBuffer[0], iBufferLength);

                std::string szFileName = skCryptDec("Update.zip");

                std::ofstream outputFile(szFileName, std::ios::out | std::ios::binary);

                if (!outputFile.is_open())
                {
                    OnUpdateDownloaded(false);
                    return;
                }

                outputFile.write(reinterpret_cast<const char*>(vecBuffer.data()), vecBuffer.size());
                outputFile.close();

                OnUpdateDownloaded(true);
            }
            else
            {
                OnUpdateDownloaded(false);
            }
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
                pkt.read(&vecBuffer[0], iBufferLength);

                try
                {
                    JSON jRouteData = JSON::parse(vecBuffer);

                    std::vector<Route> vecRoute;

                    std::string szStepListAttribute = skCryptDec("steplist");

                    for (size_t i = 0; i < jRouteData[szStepListAttribute.c_str()].size(); i++)
                    {
                        Route pRoute{};

                        std::string szXAttribute = skCryptDec("x");
                        std::string szYAttribute = skCryptDec("y");
                        std::string szStepTypeAttribute = skCryptDec("steptype");
                        std::string szPacketAttribute = skCryptDec("packet");

                        pRoute.fX = jRouteData[szStepListAttribute.c_str()][i][szXAttribute.c_str()].get<float>();
                        pRoute.fY = jRouteData[szStepListAttribute.c_str()][i][szYAttribute.c_str()].get<float>();
                        pRoute.eStepType = (RouteStepType)jRouteData[szStepListAttribute.c_str()][i][szStepTypeAttribute.c_str()].get<int>();
                        pRoute.szPacket = jRouteData[szStepListAttribute.c_str()][i][szPacketAttribute.c_str()].get<std::string>();

                        vecRoute.push_back(pRoute);
                    }

                    std::string szNameAttribute = skCryptDec("name");
                    std::string szIndexAttribute = skCryptDec("index");

                    uint8_t iIndex = jRouteData[szIndexAttribute.c_str()].get<uint8_t>();

                    auto pRouteData = m_mapRouteList.find(iIndex);

                    if (pRouteData != m_mapRouteList.end())
                    {
                        pRouteData->second.insert(std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute));
                    }
                    else
                    {
                        m_mapRouteList.insert(std::make_pair(iIndex, std::map<std::string, std::vector<Route>> {
                            std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute)
                        }));
                    }
                }
                catch (const std::exception& e)
                {
#ifdef DEBUG
                    printf("HandlePacket::Route: %s\n", e.what());
#else
                    DBG_UNREFERENCED_PARAMETER(e);
#endif
                }
            }
        }
        break;
  
    }
}

void Service::SendReady(uint32_t iCRC)
{
    Packet pkt = Packet(PacketHeader::READY);

    pkt 
        << uint32_t(GetCurrentProcessId()) 
        << uint32_t(iCRC);

    Send(pkt);
}

void Service::SendLogin(std::string szEmail, std::string szPassword)
{
    Packet pkt = Packet(PacketHeader::LOGIN);

    pkt.DByte();
    pkt
        << uint8_t(LoginType::GENERIC)
        << szEmail.c_str()
        << szPassword.c_str()
        << to_string(m_hardwareInfo.System.Name.c_str())
        << to_string(m_hardwareInfo.Registry.ComputerHardwareId.c_str())
        << to_string(m_hardwareInfo.System.OSSerialNumber.c_str());

    std::string szGPUs;
    for (size_t i = 0; i < m_hardwareInfo.GPU.size(); i++)
    {
        if (i == 0)
            szGPUs += to_string(m_hardwareInfo.GPU.at(i).Name.c_str());
        else
            szGPUs += "||" + to_string(m_hardwareInfo.GPU.at(i).Name.c_str());
    }

    pkt << szGPUs;

    Send(pkt);
}

void Service::SendLogin(std::string szToken)
{
    Packet pkt = Packet(PacketHeader::LOGIN);

    pkt.DByte();
    pkt
        << uint8_t(LoginType::TOKEN)
        << szToken.c_str()
        << to_string(m_hardwareInfo.System.Name.c_str())
        << to_string(m_hardwareInfo.Registry.ComputerHardwareId.c_str())
        << to_string(m_hardwareInfo.System.OSSerialNumber.c_str());

    std::string szGPUs;
    for (size_t i = 0; i < m_hardwareInfo.GPU.size(); i++)
    {
        if (i == 0)
            szGPUs += to_string(m_hardwareInfo.GPU.at(i).Name.c_str());
        else
            szGPUs += "||" + to_string(m_hardwareInfo.GPU.at(i).Name.c_str());
    }

    pkt << szGPUs;

    Send(pkt);

}

void Service::SendPointerRequest()
{
    Packet pkt = Packet(PacketHeader::POINTER);
    pkt 
        << uint8_t(m_ePlatformType);

    Send(pkt);
}

void Service::SendLoadUserConfiguration(uint8_t iServerId, std::string szCharacterName)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    pkt.DByte();
    pkt 
        << uint8_t(ConfigurationRequestType::LOAD) 
        << uint8_t(m_ePlatformType)
        << uint8_t(iServerId)
        << szCharacterName;

    Send(pkt);
}

void Service::SendSaveUserConfiguration(uint8_t iServerId, std::string szCharacterName)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    std::string szConfigurationData = m_iniUserConfiguration->Dump();

    pkt.DByte();
    pkt
        << uint8_t(ConfigurationRequestType::SAVE)
        << uint8_t(m_ePlatformType)
        << uint8_t(iServerId)
        << szCharacterName
        << uint32_t(szConfigurationData.size());

    pkt.append(szConfigurationData);

    Send(pkt);
}

void Service::SendInjectionRequest(uint32_t iProcessId)
{
    Packet pkt = Packet(PacketHeader::INJECTION);

    pkt.DByte();
    pkt
        << uint8_t(InjectionRequestType::REQUEST)
        << uint8_t(PlatformType::USKO)
        << uint32_t(iProcessId);

    Send(pkt);
}

void Service::SendInjectionReport(bool bStarted, uint32_t iProcessId)
{
    Packet pkt = Packet(PacketHeader::INJECTION);

    pkt.DByte();
    pkt
        << uint8_t(InjectionRequestType::REPORT)
        << uint8_t(bStarted ? 1 : 0)
        << uint32_t(iProcessId);

    Send(pkt);
}

void Service::SendPong(std::string szCharacterName, float fX, float fY, uint8_t iZoneID)
{
    Packet pkt = Packet(PacketHeader::PING);

    pkt.DByte();
    pkt << szCharacterName
        << float(fX)
        << float(fY)
        << uint8_t(iZoneID);

    Send(pkt);
}

void Service::SendCaptcha(std::vector<uint8_t> vecImageBuffer)
{
    Packet pkt = Packet(PacketHeader::CAPTCHA);

    pkt.DByte();
    pkt << uint32_t(vecImageBuffer.size());
    pkt.append(vecImageBuffer.data(), vecImageBuffer.size());

    Send(pkt);
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

    Send(pkt);
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

bool Service::GetRouteList(uint8_t iMapIndex, RouteList& pRouteList)
{
    auto pRouteData = m_mapRouteList.find(iMapIndex);

    if (pRouteData == m_mapRouteList.end())
        return false;

    pRouteList = pRouteData->second;

    return true;
}

void Service::SaveRoute(std::string szRouteName, uint8_t iMapIndex, std::vector<Route> vecRoute)
{
    try
    {
        JSON jRouteData;

        std::string szNameAttribute = skCryptDec("name");
        jRouteData[szNameAttribute.c_str()] = szRouteName;
        std::string szIndexAttribute = skCryptDec("index");
        jRouteData[szIndexAttribute.c_str()] = iMapIndex;

        for (auto& e : vecRoute)
        {
            JSON jRoute;

            std::string szXAttribute = skCryptDec("x");
            std::string szYAttribute = skCryptDec("y");
            std::string szStepTypeAttribute = skCryptDec("steptype");
            std::string szPacketAttribute = skCryptDec("packet");

            jRoute[szXAttribute.c_str()] = e.fX;
            jRoute[szYAttribute.c_str()] = e.fY;
            jRoute[szStepTypeAttribute.c_str()] = e.eStepType;
            jRoute[szPacketAttribute.c_str()] = e.szPacket;

            std::string szStepListAttribute = skCryptDec("steplist");
            jRouteData[szStepListAttribute.c_str()].push_back(jRoute);
        }

        SendRouteSaveRequest(szRouteName, jRouteData);

        auto pRouteData = m_mapRouteList.find(iMapIndex);

        if (pRouteData != m_mapRouteList.end())
        {
            pRouteData->second.insert(std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute));
        }
        else
        {
            m_mapRouteList.insert(std::make_pair(iMapIndex, std::map<std::string, std::vector<Route>> {
                std::make_pair(jRouteData[szNameAttribute.c_str()].get<std::string>(), vecRoute)
            }));
        }
    }
    catch (const std::exception& e)
    {
#ifdef DEBUG
        printf("%s\n", e.what());
#else
        DBG_UNREFERENCED_PARAMETER(e);
#endif
    }
}

void Service::DeleteRoute(std::string szRouteName, uint8_t iMapIndex)
{
    auto& inner = m_mapRouteList[iMapIndex];
    const auto it = inner.find(szRouteName);

    if (it != inner.end())
    {
        inner.erase(it);

        if (inner.size() == 0)
            m_mapRouteList.erase(iMapIndex);
    }



    //std::string routePath = skCryptDec("Data\\Route\\") + szRouteName + skCryptDec(".json");
    //remove(routePath.c_str());
}