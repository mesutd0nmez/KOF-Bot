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
    m_iniConfiguration = nullptr;
}

void Service::Initialize()
{
    PWSTR szAppDataPath = NULL;
    HRESULT hRes = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &szAppDataPath);

    if (!SUCCEEDED(hRes))
    {
#ifdef DEBUG
        printf("Cannot opened app data directory\n");
#endif
        return;
    }

    std::string szIniPath = to_string(szAppDataPath) + "\\KOF.ini";
    m_iniConfiguration = new Ini();
    m_iniConfiguration->Load(szIniPath.c_str());

    m_szToken = m_iniConfiguration->GetString("KOF", "Token", m_szToken.c_str());

    Connect("127.0.0.1", 8888);
}

void Service::OnConnect()
{
#ifdef DEBUG
    printf("Connected to socket successfully\n");
#endif
}

void Service::OnError(int32_t iErrorCode)
{
#ifdef DEBUG
    printf("Connection error: %d\n", iErrorCode);
#endif
}

void Service::OnClose(int32_t iErrorCode)
{
#ifdef DEBUG
    printf("Connection closed: %d\n", iErrorCode);
#endif
}

void Service::HandlePacket(Packet& pkt)
{
    uint8_t iHeader;

    pkt >> iHeader;

    switch (iHeader)
    {
        case PacketHeader::READY:
        {
            uint8_t iState;

            pkt >> iState;

            switch (iState)
            {
                case ReadyState::INFO:
                {
                    pkt >> m_iId;

                    SendReady();

                    GenerateSeed(m_iId + GetCurrentProcessId());
                    GetCryption()->SetInitialVector(std::to_string(GetSeed()));
                }
                break;

                case ReadyState::FINISH:
                {
#ifdef DEBUG
                    printf("Socket ready\n");
#endif

                    if (m_szToken.size() > 0)
                    {
                        SendLogin(m_szToken);
                    }

                    OnReady();
                }
                break;
            }
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
#ifdef DEBUG
                printf("Socket authenticated\n");
#endif

                SendPointerRequest();

                OnAuthenticated();
            }
            else
            {
                if (iType == LoginType::TOKEN)
                    m_szToken = m_iniConfiguration->SetString("KOF", "Token", "");
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

                    pkt >> szConfiguration;

                    m_iniUserConfiguration = new Ini();
                    m_iniUserConfiguration->Load(szConfiguration);

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
            SendPong();

            OnPong();
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
    HardwareID pHardwareID;

    pHardwareID.GetHardwareID();

    std::string szSystemName;
    std::transform(pHardwareID.System.Name.begin(), pHardwareID.System.Name.end(), std::back_inserter(szSystemName), [](wchar_t c)
    {
        return (char)c;
    });

    std::string szSerialNumber;
    std::transform(pHardwareID.SMBIOS.SerialNumber.begin(), pHardwareID.SMBIOS.SerialNumber.end(), std::back_inserter(szSerialNumber), [](wchar_t c)
    {
        return (char)c;
    });

    std::string szProcessorID;
    std::transform(pHardwareID.CPU.ProcessorID.begin(), pHardwareID.CPU.ProcessorID.end(), std::back_inserter(szProcessorID), [](wchar_t c)
    {
        return (char)c;
    });

    std::string szComputerHardwareID;
    std::transform(pHardwareID.Registry.ComputerHardwareID.begin(), pHardwareID.Registry.ComputerHardwareID.end(), std::back_inserter(szComputerHardwareID), [](wchar_t c)
    {
        return (char)c;
    });

    Packet pkt = Packet(PacketHeader::LOGIN);

    pkt.DByte();
    pkt
        << uint8_t(LoginType::TOKEN)
        << szToken.c_str()
        << szSystemName.c_str()
        << szSerialNumber.c_str()
        << szProcessorID.c_str()
        << szComputerHardwareID.c_str();

    Send(pkt);
}

void Service::SendPointerRequest()
{
    Packet pkt = Packet(PacketHeader::POINTER);
    pkt << uint8_t(PlatformType::CNKO);

    Send(pkt);
}

void Service::SendPong()
{
    Packet pkt = Packet(PacketHeader::PING);

    pkt.DByte();

    Send(pkt);
}

void Service::SendLoadUserConfiguration(uint8_t iServerId, std::string szCharacterName)
{
    Packet pkt = Packet(PacketHeader::CONFIGURATION);

    pkt.DByte();
    pkt 
        << uint8_t(ConfigurationRequestType::LOAD) 
        << uint8_t(AppType::BOT) 
        << uint8_t(PlatformType::CNKO) 
        << uint8_t(1) 
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
        << uint8_t(PlatformType::CNKO) 
        << uint8_t(1) 
        << szCharacterName 
        << m_iniUserConfiguration->Dump();

    Send(pkt, true);
}