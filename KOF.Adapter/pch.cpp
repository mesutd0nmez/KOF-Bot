// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

#pragma comment(lib,"ws2_32.lib")

DWORD Read4Byte(DWORD iAdress)
{
    return *(DWORD*)(iAdress);
}

DWORD HookFunction(LPCSTR szModuleName, LPCSTR szFuncName, LPVOID pHookFunc, LPVOID* pTrampolineFunc, const int iLen)
{
    *pTrampolineFunc = VirtualAlloc(NULL, (iLen + 5), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    HMODULE hModule = GetModuleHandle(szModuleName);

    if (!hModule)
    {
        return 0;
    }

    DWORD dwBaseAddr = (DWORD)GetProcAddress(hModule, szFuncName);

    DWORD dwOldProtection;
    VirtualProtect((LPVOID)dwBaseAddr, iLen, PAGE_EXECUTE_READWRITE, &dwOldProtection);

    DWORD retJmpAddr = (dwBaseAddr - (DWORD)*pTrampolineFunc) - iLen;

    BYTE byTrampoline[60];

    memcpy(byTrampoline, (LPVOID)dwBaseAddr, iLen);
    byTrampoline[iLen] = 0xE9;
    memcpy(&byTrampoline[iLen + 1], &retJmpAddr, 4);
    memcpy(*pTrampolineFunc, byTrampoline, iLen + 5);

    BYTE byJMP[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };

    DWORD dwProxy = ((DWORD)pHookFunc - dwBaseAddr) - 5;
    memcpy(&byJMP[1], &dwProxy, 4);
    memcpy((LPVOID)dwBaseAddr, byJMP, iLen);

    if (iLen > 5)
    {
        for (int i = 0; i < (iLen - 5); i++)
        {
            memset((LPVOID)((dwBaseAddr + 5) + i), 0x90, 1);
        }
    }

    VirtualProtect((LPVOID)dwBaseAddr, iLen, dwOldProtection, &dwOldProtection);
    FlushInstructionCache(GetCurrentProcess(), NULL, NULL);

    return dwBaseAddr;
}

