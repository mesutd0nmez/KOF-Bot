#include "pch.h"
#include "Service.h"
#include "HardwareID.h"

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
    m_iSelectedAccount = 0;

    m_iAccountTypeFlag = AccountTypeFlag::ACCOUNT_TYPE_FLAG_FREE;

    m_isServiceClosed = false;
}

void Service::Initialize()
{
    std::string szIniPath = skCryptDec(".\\data\\KOF.ini");

    m_iniAppConfiguration = new Ini();
    m_iniAppConfiguration->Load(szIniPath.c_str());

    m_szToken = m_iniAppConfiguration->GetString(skCryptDec("KOF"), skCryptDec("Token"), m_szToken.c_str());

//#ifdef DEBUG
//    Connect(skCryptDec("127.0.0.1"), 8888);
//#else
    Connect(skCryptDec("watchdog.kofbot.com"), 8888);
//#endif 
}

void Service::OnConnect()
{
#ifdef DEBUG
    printf("Connected to socket successfully\n");
#endif
    SendReady();
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

#ifdef DEBUG
            printf("Socket ready\n");
#endif

            GenerateSeed(m_iId + GetCurrentProcessId());
            m_Cryption->SetInitialVector(std::to_string(GetSeed()));

            OnReady();
        }
        break;

        case PacketHeader::LOGIN:
        {
            if (m_iId == -1) return;

            uint8_t iType, iStatus;

            pkt.DByte();
            pkt >> iType >> iStatus;

            if (iStatus == 1)
            {
                pkt >> m_iAccountTypeFlag;
            }

            if (iStatus == 1)
            {
#ifdef DEBUG
                printf("Socket authenticated\n");
#endif

                SendPointerRequest();

                OnAuthenticated();
            }
            else
            {
#ifdef DEBUG
                printf("Socket authentication failed\n");
#endif
            }
        }
        break;

        case PacketHeader::CONFIGURATION:
        {
            if (m_iId == -1) return;

            uint8_t iType;

            pkt.DByte();
            pkt >> iType;

            switch (iType)
            {
                case ConfigurationRequestType::LOAD:
                {
                    if (m_iId == -1) return;

                    std::string szConfiguration;
                    pkt.readString(szConfiguration);

                    m_iniUserConfiguration = new Ini();

                    if (szConfiguration.size() > 0)
                    {
                        m_iniUserConfiguration->Load(szConfiguration);
                    }                

                    OnConfigurationLoaded();
                }
                break;
            }
        }
        break;

        case PacketHeader::POINTER:
        {
            if (m_iId == -1) return;

            int iBufferLength;

            pkt >> iBufferLength;

            std::vector<uint8_t> vecBuffer(iBufferLength);
            pkt.read(&vecBuffer[0], iBufferLength);

            std::string szData((char*)vecBuffer.data(), vecBuffer.size());

            m_iniPointer = new Ini();
            m_iniPointer->Load(szData);

            OnLoaded();
        }
        break;

        case PacketHeader::PING:
        {
            uint32_t iLastPingTime;

            pkt.DByte();
            pkt >> iLastPingTime;

            SendPong();
            OnPong();
        }
        break;

        case PacketHeader::INJECTION:
        {
            if (m_iId == -1) return;

            int32_t iProcesssId, iBufferLength;

            pkt >> iProcesssId >> iBufferLength;

            std::vector<uint8_t> vecBuffer(iBufferLength);
            pkt.read(&vecBuffer[0], iBufferLength);

            Injection(iProcesssId, vecBuffer);
        }
        break;
    }
}

void Service::SendReady()
{
    Packet pkt = Packet(PacketHeader::READY);
    pkt << uint32_t(GetCurrentProcessId());

    Send(pkt);
}

void Service::SendLogin(std::string szToken)
{
    HardwareId HWID{};

    Packet pkt = Packet(PacketHeader::LOGIN);

    pkt.DByte();
    pkt
        << uint8_t(LoginType::TOKEN)
        << szToken.c_str()
        << to_string(HWID.System.Name)
        << to_string(HWID.CPU.ProcessorId)
        << to_string(HWID.SMBIOS.SerialNumber);

    std::string szHddSerial;
    for (size_t i = 0; i < HWID.Disk.size(); i++)
    {
        HardwareId::DiskObject& Disk{ HWID.Disk.at(i) };

        if (Disk.IsBootDrive && (HWID.Disk.at(i).MediaType == 3 || HWID.Disk.at(i).MediaType == 4))
        {
            if (i == 0)
                szHddSerial += to_string(Disk.SerialNumber);
            else
                szHddSerial += "||" + to_string(Disk.SerialNumber);
        }
    }

    pkt
        << szHddSerial
        << to_string(HWID.Registry.ComputerHardwareId)
        << to_string(HWID.System.OSSerialNumber);

    std::string szPartNumber;
    for (size_t i = 0; i < HWID.PhysicalMemory.size(); i++)
    {
        HardwareId::PhysicalMemoryObject& Memory{ HWID.PhysicalMemory.at(i) };

        if (i == 0)
            szPartNumber += to_string(Memory.PartNumber);
        else
            szPartNumber += "||" + to_string(Memory.PartNumber);
    }

    pkt
        << szPartNumber;

    std::string szGPUs;
    for (size_t i = 0; i < HWID.GPU.size(); i++)
    {
        if (i == 0)
            szGPUs += to_string(HWID.GPU.at(i).Name);
        else
            szGPUs += "||" + to_string(HWID.GPU.at(i).Name);
    }

    pkt << szGPUs;

    Send(pkt);
}

void Service::SendPointerRequest()
{
    Packet pkt = Packet(PacketHeader::POINTER);
    pkt 
        << uint8_t(AppType::BOT)
        << uint8_t(m_ePlatformType);

    Send(pkt);
}

void Service::SendPong()
{
    Packet pkt = Packet(PacketHeader::PING);

    pkt.DByte();
    pkt << uint32_t(time(0));

    Send(pkt);
}

void Service::SendLoadUserConfiguration(uint8_t iServerId, std::string szCharacterName)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    pkt.DByte();
    pkt 
        << uint8_t(ConfigurationRequestType::LOAD) 
        << uint8_t(AppType::BOT) 
        << uint8_t(m_ePlatformType)
        << uint8_t(1) //server index
        << szCharacterName;

    Send(pkt);
}

void Service::SendSaveUserConfiguration(uint8_t iServerId, std::string szCharacterName)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    pkt.DByte();
    pkt 
        << uint8_t(ConfigurationRequestType::SAVE) 
        << uint8_t(AppType::BOT) 
        << uint8_t(m_ePlatformType)
        << uint8_t(1) //server index
        << szCharacterName 
        << m_iniUserConfiguration->Dump();

    Send(pkt, true);
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

#ifdef DEBUG
    printf("Service: Injection request sended\n");
#endif
}