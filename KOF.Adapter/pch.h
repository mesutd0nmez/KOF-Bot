// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include "../KOF.Bot/Define.h"
#include "SkCrypter.h"
#include "Enum.h"
#include "Packet.h"

#define Print(a, ...) printf("%s: " a "\n", __func__, ##__VA_ARGS__)

typedef int (WINAPI* WSAConnectTrampoline)(SOCKET s, const struct sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);

struct SkillQueue
{
    uint32_t iSkillID;
    int32_t iTargetID;
    int32_t iPriority;
    uint8_t iAttacking;
    uint8_t iBasicAttack;

    bool operator<(const SkillQueue& rhs) const
    {
        return iPriority < rhs.iPriority;
    }
};

static std::map<std::string, DWORD> m_mapAddress;

static std::queue<bool> m_qSelectCharacterSkipQueue;
static std::queue<bool> m_qSelectCharacterLeftQueue;
static std::queue<bool> m_qSelectCharacterRightQueue;
static std::queue<bool> m_qSelectCharacterQueue;
static std::queue<bool> m_qLoginRequestQueue;
static std::queue<bool> m_qBasicAttackQueue;
static std::priority_queue<SkillQueue> m_qSkillQueue;
static std::queue<bool> m_qLoadServerListQueue;
static std::queue<int> m_qSelectServerQueue;
static std::queue<bool> m_qShowChannelQueue;
static std::queue<int> m_qSelectChannelQueue;
static std::queue<bool> m_qConnectServerQueue;

static DWORD bLastAttackableTargetBase;
static DWORD iSaveCPUSleepTime;
static LPVOID pWSAConnectTrampoline;

// Proxy Info
static int szProxyPort;
static std::string szProxyAddress;
static std::string szProxyUsername;
static std::string szProxyPassword;

extern DWORD Read4Byte(DWORD lpAdress);
extern DWORD HookFunction(LPCSTR szModuleName, LPCSTR szFuncName, LPVOID pHookFunc, LPVOID* pTrampolineFunc, const int iLen);

#endif //PCH_H
