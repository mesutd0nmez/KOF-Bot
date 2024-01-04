#include "pch.h"
#include <stdio.h>

BOOL g_bWorking = FALSE;

typedef void(__thiscall* Send)(DWORD, uint8_t*, uint32_t);

DWORD GetAddress(std::string szAddressName)
{
    auto it = m_mapAddress.find(szAddressName);

    if (it != m_mapAddress.end())
        return it->second;

    return 0;
}

void Login()
{
    DWORD iIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
    DWORD iCall1 = GetAddress(skCryptDec("KO_PTR_LOGIN_REQUEST1"));
    DWORD iCall2 = GetAddress(skCryptDec("KO_PTR_LOGIN_REQUEST2"));

    __asm
    {
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EDI, iCall1
        CALL EDI
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV ESI, iCall2
        CALL ESI
    }
}

void LoadServerList()
{
    DWORD iIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_LOAD_SERVER_LIST"));

    __asm
    {
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV ECX, [ECX + 0x2C]
        MOV EDI, iCall
        CALL EDI
    }
}

void SelectServer(int iIndex)
{
    DWORD iIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_SERVER_SELECT"));

    __asm
    {
        PUSH iIndex
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV ECX, [ECX + 0x2C]
        MOV EDI, iCall
        CALL EDI
    }
}

void ShowChannel()
{
    DWORD iIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_SHOW_CHANNEL"));

    __asm
    {
        PUSH 0xCD
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV ECX, [ECX + 0x2C]
        MOV EDI, iCall
        CALL EDI
    }
}

void SelectChannel(int iIndex)
{
    DWORD iIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_SELECT_CHANNEL"));

    __asm
    {
        PUSH iIndex
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV ECX, [ECX + 0x2C]
        MOV EDI, iCall
        CALL EDI
    }
}

void ConnectServer()
{
    DWORD iIntro = GetAddress(skCryptDec("KO_PTR_INTRO"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_CONNECT_SERVER"));
    DWORD iConnectNeed = GetAddress(skCryptDec("KO_PTR_CONNECT_SERVER_NEED"));

    __asm
    {
        MOV ECX, iIntro
        MOV ECX, DWORD PTR DS : [ECX]
        MOV ESI, iConnectNeed
        MOV DWORD PTR DS : [ESI] , 1
        MOV EDI, iCall
        CALL EDI
    }
}

void SelectCharacterSkip()
{
    DWORD iSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_SKIP"));

    __asm
    {
        MOV ECX, iSelect
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EDI, iCall
        CALL EDI
    }
}

void SelectCharacterLeft()
{
    DWORD iSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_LEFT"));

    __asm
    {
        MOV ECX, iSelect
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EDI, iCall
        CALL EDI
    }
}

void SelectCharacterRight()
{
    DWORD iSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_RIGHT"));

    __asm
    {
        MOV ECX, iSelect
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EDI, iCall
        CALL EDI
    }
}

void SelectCharacter()
{
    DWORD iSelect = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT"));
    DWORD iCall = GetAddress(skCryptDec("KO_PTR_CHARACTER_SELECT_ENTER"));

    __asm
    {
        MOV ECX, iSelect
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EDI, iCall
        CALL EDI
    }
}

bool IsEnemy(DWORD iBase)
{
    DWORD iCHR = GetAddress(skCryptDec("KO_PTR_CHR"));
    DWORD iEnemy = GetAddress(skCryptDec("KO_PTR_ENEMY"));

    DWORD iEnemyBase = 0;

    __asm
    {
        MOV ECX, iCHR
        MOV ECX, DWORD PTR DS : [ECX]
        PUSH iBase
        MOV EAX, iEnemy
        CALL EAX
        MOV iEnemyBase, EAX
    }

    return iEnemyBase > 0;
}

DWORD GetEntityBase(int32_t iTargetID)
{
    DWORD iFLDB = GetAddress(skCryptDec("KO_PTR_FLDB"));
    DWORD iFMBS = GetAddress(skCryptDec("KO_PTR_FMBS"));

    DWORD iMobBase = 0;

    __asm
    {
        MOV ECX, iFLDB
        MOV ECX, DWORD PTR DS : [ECX]
        PUSH 0x01
        PUSH iTargetID
        MOV EAX, iFMBS
        CALL EAX
        MOV iMobBase, EAX
    }

    return iMobBase;
}

void BasicAttack()
{
    DWORD iDLG = GetAddress(skCryptDec("KO_PTR_DLG"));
    DWORD iLRCA = GetAddress(skCryptDec("KO_LRCA"));

    __asm
    {
        MOV ECX, iDLG
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EAX, iLRCA
        CALL EAX
    }
}

DWORD GetSkillBase(uint32_t iSkillID)
{
    DWORD iSBEC = GetAddress(skCryptDec("KO_SBEC"));
    DWORD iSBCA = GetAddress(skCryptDec("KO_SBCA"));

    DWORD iSkillBase = 0;

    __asm
    {
        PUSH iSkillID
        MOV ECX, iSBEC
        MOV ECX, DWORD PTR DS : [ECX]
        MOV EAX, iSBCA
        CALL EAX
        MOV iSkillBase, EAX
    }

    return iSkillBase;
}

void UseSkill(int32_t iTargetID, uint32_t iSkillID)
{
    DWORD iSkillBase = GetSkillBase(iSkillID);
    DWORD iSkillManager = Read4Byte(Read4Byte(GetAddress(skCryptDec("KO_PTR_DLG"))) + GetAddress(skCryptDec("KO_OFF_LPEO")));
    DWORD iLSCA = GetAddress(skCryptDec("KO_LSCA"));

    if (iSkillBase == 0)
        return;

    __asm
    {
        PUSH iSkillBase
        PUSH iTargetID
        MOV ECX, iSkillManager
        MOV EAX, iLSCA
        CALL EAX
    }
}

void GameProcMainProcess()
{
    if (!m_qLoginRequestQueue.empty())
    {
        Login();
        m_qLoginRequestQueue.pop();
    }

    if (!m_qLoadServerListQueue.empty())
    {
        LoadServerList();
        m_qLoadServerListQueue.pop();
    }

    if (!m_qSelectServerQueue.empty())
    {
        int iIndex = m_qSelectServerQueue.front();
        SelectServer(iIndex);
        m_qSelectServerQueue.pop();
    }

    if (!m_qShowChannelQueue.empty())
    {
        ShowChannel();
        m_qShowChannelQueue.pop();
    }

    if (!m_qSelectChannelQueue.empty())
    {
        int iIndex = m_qSelectChannelQueue.front();
        SelectChannel(iIndex);
        m_qSelectChannelQueue.pop();
    }

    if (!m_qConnectServerQueue.empty())
    {
        ConnectServer();
        m_qConnectServerQueue.pop();
    }

    if (!m_qSelectCharacterSkipQueue.empty())
    {
        SelectCharacterSkip();
        m_qSelectCharacterSkipQueue.pop();
    }

    if (!m_qSelectCharacterLeftQueue.empty())
    {
        SelectCharacterLeft();
        m_qSelectCharacterLeftQueue.pop();
    }

    if (!m_qSelectCharacterRightQueue.empty())
    {
        SelectCharacterRight();
        m_qSelectCharacterRightQueue.pop();
    }

    if (!m_qSelectCharacterQueue.empty())
    {
        SelectCharacter();
        m_qSelectCharacterQueue.pop();
    }

    if (!m_qSkillQueue.empty())
    {
        SkillQueue sSkillQueue = m_qSkillQueue.top();
        UseSkill(sSkillQueue.iTargetID, sSkillQueue.iSkillID);
        m_qSkillQueue.pop();
    }

    if (!m_qBasicAttackQueue.empty())
    {
        BasicAttack();
        m_qBasicAttackQueue.pop();
    }
}

void SaveCPUProcess()
{
    if (iSaveCPUSleepTime > 0)
    {
        Sleep(iSaveCPUSleepTime * 2 / 5);
    }
}

void PatchSaveCPUThread()
{
    BYTE byPatch1[] =
    {
        0x60,							//pushad
        0xBA, 0x00, 0x00, 0x00, 0x00,	//mov edx,00000000 <-- Hook Main Thread Address
        0xFF, 0xD2,						//call edx
        0x61,							//popad
        0x55,                           //push ebp
        0x8B, 0xEC,                     //mov ebp,esp
        0x83, 0xEC, 0x08,               //sub esp,08
        0xBA, 0x00, 0x00, 0x00, 0x00,	//mov edx,00000000 <-- Main Thread Address
        0x83, 0xC2, 0x06,				//add edx,06
        0xFF, 0xE2						//jmp edx
    };

    DWORD pUIManagerProcess = (DWORD)(LPVOID*)SaveCPUProcess;
    memcpy(byPatch1 + 2, &pUIManagerProcess, sizeof(pUIManagerProcess));

    DWORD pMainThread = GetAddress(skCryptDec("KO_PTR_SAVE_CPU"));
    memcpy(byPatch1 + 16, &pMainThread, sizeof(pMainThread));

    LPVOID pPatchAddress = VirtualAllocEx(GetCurrentProcess(), 0, sizeof(byPatch1), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (pPatchAddress == 0)
        return;

    WriteProcessMemory(GetCurrentProcess(), (LPVOID)pPatchAddress, &byPatch1[0], sizeof(byPatch1), 0);

    BYTE byPatch2[] =
    {
        0xE9, 0x00, 0x00, 0x00, 0x00,
        0x90,
    };

    DWORD iCallDifference = ((DWORD)pPatchAddress - GetAddress(skCryptDec("KO_PTR_SAVE_CPU")) - 5);
    memcpy(byPatch2 + 1, &iCallDifference, sizeof(iCallDifference));

    DWORD iOldProtection;
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)GetAddress(skCryptDec("KO_PTR_SAVE_CPU")), sizeof(byPatch2), PAGE_EXECUTE_READWRITE, &iOldProtection);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)GetAddress(skCryptDec("KO_PTR_SAVE_CPU")), &byPatch2[0], sizeof(byPatch2), 0);
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)GetAddress(skCryptDec("KO_PTR_SAVE_CPU")), sizeof(byPatch2), iOldProtection, &iOldProtection);

#ifdef DEBUG_LOG
    Print("Save CPU Thread Patched");
#endif
}

void PatchGameProcMainThread()
{
    BYTE byPatch1[] =
    {
        0x60,							//pushad
        0xBA, 0x00, 0x00, 0x00, 0x00,	//mov edx,00000000 <-- Hook Main Thread Address
        0xFF, 0xD2,						//call edx
        0x61,							//popad
        0xBA, 0x00, 0x00, 0x00, 0x00,	//mov edx,00000000 <-- Main Thread Address
        0x83, 0xC2, 0x5,				//add edx,05
        0xFF, 0xE2						//jmp edx
    };

    DWORD pGameProcMainProcess = (DWORD)(LPVOID*)GameProcMainProcess;
    memcpy(byPatch1 + 2, &pGameProcMainProcess, sizeof(pGameProcMainProcess));

    DWORD pMainThread = GetAddress(skCryptDec("KO_PTR_MAIN_THREAD"));
    memcpy(byPatch1 + 10, &pMainThread, sizeof(pMainThread));

    LPVOID pPatchAddress = VirtualAllocEx(GetCurrentProcess(), 0, sizeof(byPatch1), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (pPatchAddress == 0)
        return;

    WriteProcessMemory(GetCurrentProcess(), (LPVOID)pPatchAddress, &byPatch1[0], sizeof(byPatch1), 0);

    BYTE byPatch2[] =
    {
        0xE9, 0x00, 0x00, 0x00, 0x00
    };

    DWORD iCallDifference = ((DWORD)pPatchAddress - GetAddress(skCryptDec("KO_PTR_MAIN_THREAD")) - 5);
    memcpy(byPatch2 + 1, &iCallDifference, sizeof(iCallDifference));

    DWORD iOldProtection;
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)GetAddress(skCryptDec("KO_PTR_MAIN_THREAD")), sizeof(byPatch2), PAGE_EXECUTE_READWRITE, &iOldProtection);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)GetAddress(skCryptDec("KO_PTR_MAIN_THREAD")), &byPatch2[0], sizeof(byPatch2), 0);
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)GetAddress(skCryptDec("KO_PTR_MAIN_THREAD")), sizeof(byPatch2), iOldProtection, &iOldProtection);

#ifdef DEBUG_LOG
    Print("Main Thread Patched");
#endif
}

int WINAPI WSAConnectHook(SOCKET s, const struct sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS)
{
    SHORT PORT = 0;
    BYTE IP[4]{};

    int iResult = SOCKET_ERROR;

    memcpy(IP, name->sa_data + 2, 4);
    PORT = htons(((sockaddr_in*)name)->sin_port);

    inet_pton(AF_INET, szProxyAddress.c_str(), &((sockaddr_in*)name)->sin_addr);
    ((sockaddr_in*)name)->sin_port = htons(szProxyPort);

    // 1 - Connect Proxy Server
    iResult = ((WSAConnectTrampoline)pWSAConnectTrampoline)(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);

    // en.wikipedia.org/wiki/SOCKS - #Client greeting
    BYTE byGreetingMessage[] = { 0x05, 0x01, 0x00 };

    if (szProxyUsername.size() > 0 || szProxyPassword.size() > 0)
        byGreetingMessage[2] = 0x02;

    // 2 - Send Proxy Greeting Message
    const auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );

    do
    {
        std::chrono::milliseconds msNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        );

        if ((startTime.count() + (30 * 1000)) < msNow.count())
        {
            WSACleanup();
            break;
        }

        iResult = send(s, (const char*)byGreetingMessage, sizeof(byGreetingMessage), 0);
    } while (iResult == SOCKET_ERROR);

    // 3 - Initialize Proxy Response
    BYTE byStreamBuffer[0x400]{};
    int iStreamLength = 0;

    bool bStreamHandshakeComplete = false;

    while (iResult != SOCKET_ERROR
        && !bStreamHandshakeComplete)
    {
        iStreamLength = recv(s, (char*)byStreamBuffer, 0x400, 0);

        if (iStreamLength > 0)
        {
#ifdef DEBUG_LOG
            Print("Proxy Recv Size: %d", iStreamLength);
            printf("Proxy Recv: ");

            for (int i = 0; i < iStreamLength; i++)
                printf("%02x", byStreamBuffer[i]);

            printf("\n");

#endif
            if ((byStreamBuffer[0] == 0x05 || byStreamBuffer[0] == 0x01)
                && byStreamBuffer[1] == 0x00 && iStreamLength == 2)
            {
                // Create a vector for the proxy connection message
                std::vector<BYTE> vecConnectionMessage(10);

                // SOCKS5
                vecConnectionMessage[0] = 0x05;
                vecConnectionMessage[1] = 0x01;
                vecConnectionMessage[2] = 0x00;
                vecConnectionMessage[3] = 0x01;

                // Copy the target IP address into the vector
                memcpy(vecConnectionMessage.data() + 4, IP, 4);

                // Set the target port
                vecConnectionMessage[8] = PORT >> 8; // Most significant byte
                vecConnectionMessage[9] = PORT & 0xFF; // Least significant byte

                // Send the proxy connection message
                if (send(s, reinterpret_cast<const char*>(vecConnectionMessage.data()), 10, 0) == SOCKET_ERROR)
                {
#ifdef DEBUG_LOG
                    Print("Proxy server connected but target information send failed");
#endif
                    WSACleanup();
                    break;
                }
                else
                {
#ifdef DEBUG_LOG
                    Print("Proxy connection successful, target information being sent");
#endif
                }
            }
            else if (byStreamBuffer[0] == 0x05 && byStreamBuffer[1] == 0x02)
            {
                // We're using std::vector to manage dynamic memory allocation automatically.
                std::vector<BYTE> vecAuthenticationMessage;

                // Adding protocol version and username length.
                vecAuthenticationMessage.push_back(0x01);
                vecAuthenticationMessage.push_back(static_cast<BYTE>(szProxyUsername.length()));

                // Appending the username to the vector.
                vecAuthenticationMessage.insert(vecAuthenticationMessage.end(), szProxyUsername.begin(), szProxyUsername.end());

                // Adding the password length.
                vecAuthenticationMessage.push_back(static_cast<BYTE>(szProxyPassword.length()));

                // Appending the password to the vector.
                vecAuthenticationMessage.insert(vecAuthenticationMessage.end(), szProxyPassword.begin(), szProxyPassword.end());

                // Sending the prepared data.
                if (send(s, reinterpret_cast<const char*>(vecAuthenticationMessage.data()), vecAuthenticationMessage.size(), 0) == SOCKET_ERROR)
                {
#ifdef DEBUG_LOG
                    Print("Proxy authentication information send failed");
#endif
                    WSACleanup();
                    break;
                }
                else
                {
#ifdef DEBUG_LOG
                    Print("Proxy authentication information sended");
#endif
                }
            }
            else if (byStreamBuffer[0] == 0x05 && byStreamBuffer[1] == 0x00)
            {
                bStreamHandshakeComplete = true;

#ifdef DEBUG_LOG
                Print("Proxy handshake completed");
#endif
                break;
            }
        }
    }

    return iResult;
}

void SetProxyInformation(std::string szIP, int iPort, std::string szUsername = "", std::string szPassword = "")
{
    ::szProxyAddress = szIP;
    ::szProxyPort = iPort;
    ::szProxyUsername = szUsername;
    ::szProxyPassword = szPassword;
}

void StartMailslot()
{
#ifdef DEBUG_LOG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    Print("Internal connection starting");
#endif

    std::string szMailslotName = skCryptDec("\\\\.\\mailslot\\Internal\\") + std::to_string(GetCurrentProcessId());
    HANDLE hMailslot = CreateMailslot(szMailslotName.c_str(), 0, MAILSLOT_WAIT_FOREVER, nullptr);

    if (hMailslot == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG_LOG
        Print("Failed to create mailslot. Exiting");
#endif
        return;
    }

#ifdef DEBUG_LOG
    Print("Internal connection ready");
#endif

    while (g_bWorking)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        DWORD iCurrentMesageSize, iMesageLeft, iMessageReadSize;

        BOOL bResult = GetMailslotInfo(hMailslot, NULL, &iCurrentMesageSize, &iMesageLeft, NULL);

        if (!bResult)
            continue;

        if (iCurrentMesageSize == MAILSLOT_NO_MESSAGE)
            continue;

        std::vector<uint8_t> vecMessageBuffer;
        vecMessageBuffer.resize(iCurrentMesageSize);

        OVERLAPPED ov{};
        ov.Offset = 0;
        ov.OffsetHigh = 0;
        ov.hEvent = NULL;

        if (ReadFile(hMailslot, &vecMessageBuffer[0], iCurrentMesageSize, &iMessageReadSize, &ov))
        {
            vecMessageBuffer.resize(iMessageReadSize);

            Packet pkt = Packet(vecMessageBuffer[0], vecMessageBuffer.size());
            pkt.append(&vecMessageBuffer[1], vecMessageBuffer.size() - 1);

            uint8_t iHeader;
            pkt >> iHeader;

            switch (iHeader)
            {
                case PIPE_LOAD_POINTER:
                {
                    int32_t iAddressCount;
                    pkt >> iAddressCount;

                    for (int32_t i = 0; i < iAddressCount; i++)
                    {
                        std::string szAddressName;
                        DWORD iAddress;

                        pkt >> szAddressName >> iAddress;

    #ifdef DEBUG_LOG
                        Print("%s=%d", szAddressName.c_str(), iAddress);
    #endif

                        m_mapAddress.insert(std::make_pair(szAddressName, iAddress));
                    }

                    if (iAddressCount > 0)
                    {
                        iSaveCPUSleepTime = 0;

                        PatchGameProcMainThread();
                        PatchSaveCPUThread();
                    }
                }
                break;

                case PIPE_BASIC_ATTACK:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qBasicAttackQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Basic Attack: %d", iEnable);
    #endif
                }
                break;

                case PIPE_USE_SKILL:
                {
                    SkillQueue sSkillQueue;

                    pkt
                        >> sSkillQueue.iTargetID
                        >> sSkillQueue.iSkillID
                        >> sSkillQueue.iPriority
                        >> sSkillQueue.iAttacking
                        >> sSkillQueue.iBasicAttack;

    #ifdef DEBUG_LOG
                    Print("Use Skill: %d, %d, %d, %d, %d",
                        sSkillQueue.iTargetID, 
                        sSkillQueue.iSkillID, 
                        sSkillQueue.iPriority, 
                        sSkillQueue.iAttacking, 
                        sSkillQueue.iBasicAttack);
    #endif

                    m_qSkillQueue.push(sSkillQueue);

                }
                break;

                case PIPE_LOGIN:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qLoginRequestQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Login Requested: %d", iEnable);
    #endif
                }
                break;

                case PIPE_SELECT_CHARACTER_SKIP:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qSelectCharacterSkipQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Select character skip: %d", iEnable);
    #endif
                }
                break;

                case PIPE_SELECT_CHARACTER_LEFT:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qSelectCharacterLeftQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Select character left: %d", iEnable);
    #endif
                }
                break;

                case PIPE_SELECT_CHARACTER_RIGHT:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qSelectCharacterRightQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Select character right: %d", iEnable);
    #endif
                }
                break;

                case PIPE_SELECT_CHARACTER_ENTER:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qSelectCharacterQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Select character enter: %d", iEnable);
    #endif
                }
                break;

                case PIPE_LOAD_SERVER_LIST:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qLoadServerListQueue.push(true);

    #ifdef DEBUG_LOG
                    Print("Load Server List: %d", iEnable);
    #endif
                }
                break;

                case PIPE_SELECT_SERVER:
                {
                    uint8_t iIndex;
                    pkt >> iIndex;

                    m_qSelectServerQueue.push(iIndex);

    #ifdef DEBUG_LOG
                    Print("Select Server Index: %d", iIndex);
    #endif
                }
                break;

                case PIPE_SHOW_CHANNEL:
                {
                    uint8_t iEnable;
                    pkt >> iEnable;

                    m_qShowChannelQueue.push(iEnable);

    #ifdef DEBUG_LOG
                    Print("Show Channel: %d", iEnable);
    #endif
                }
                break;

                case PIPE_SELECT_CHANNEL:
                {
                    uint8_t iIndex;
                    pkt >> iIndex;

                    m_qSelectChannelQueue.push(iIndex);

    #ifdef DEBUG_LOG
                    Print("Select Channel: %d", iIndex);
    #endif
                }
                break;

                case PIPE_CONNECT_SERVER:
                {
                    uint8_t iIndex;
                    pkt >> iIndex;

                    m_qConnectServerQueue.push(iIndex);

    #ifdef DEBUG_LOG
                    Print("Connect Server: %d", iIndex);
    #endif
                }
                break;

                case PIPE_SAVE_CPU:
                {
                    int32_t iValue;
                    pkt >> iValue;

                    iSaveCPUSleepTime = iValue;

    #ifdef DEBUG_LOG
                    Print("Save CPU Value: %d", iValue);
    #endif
                }
                break;

                case PIPE_PROXY:
                {
                    std::string szProxyIP, szUsername, szPassword;
                    int32_t iProxyPort;

                    pkt >> szProxyIP >> iProxyPort >> szUsername >> szPassword;

#ifdef DEBUG_LOG
                    Print("Proxy Information: %s:%d | %s | %s", szProxyIP.c_str(), iProxyPort, szUsername.c_str(), szPassword.c_str());
#endif

                    SetProxyInformation(szProxyIP, iProxyPort, szUsername, szPassword);
                    HookFunction(skCryptDec("ws2_32.dll"), skCryptDec("WSAConnect"), (LPVOID*)WSAConnectHook, &pWSAConnectTrampoline, 5);
                }
                break;

                case PIPE_SEND_PACKET:
                {
                    Send pSendFnc = (Send)GetAddress("KO_SND_FNC");
                    pSendFnc(*reinterpret_cast<DWORD*>(GetAddress("KO_PTR_PKT")), pkt.contents() + 2, pkt.size());
                }
                break;
            }
        }
        else
        {
#ifdef DEBUG_LOG
            Print("Error reading from mailslot. Exiting");
#endif
            break;
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);

            g_bWorking = TRUE;
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&StartMailslot, 0, 0, 0);
        }
        break;

        case DLL_PROCESS_DETACH:
            g_bWorking = FALSE;
            break;
    }

    return TRUE;
}

