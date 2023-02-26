#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#define CopyBytes(Dest,Src) memcpy(Dest, (BYTE*)&Src, sizeof(Src))

class Memory
{
public:
    inline static BYTE ReadByte(HANDLE hProcess, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(BYTE)))
        {
            return(*(BYTE*)(dwAddress));
        }

        return 0;
#else
        BYTE byValue;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &byValue, sizeof(BYTE), 0);
        return byValue;
#endif
    }

    inline static DWORD Read4Byte(HANDLE hProcess, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(DWORD)))
        {
            return(*(DWORD*)(dwAddress));
        }

        return 0;
#else
        DWORD dwValue;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &dwValue, sizeof(DWORD), 0);
        return dwValue;
#endif
    }

    inline static float ReadFloat(HANDLE hProcess, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(float)))
        {
            return(*(float*)(dwAddress));
        }

        return 0;
#else
        float fValue;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &fValue, sizeof(float), 0);
        return fValue;
#endif
    }

    inline static std::string ReadString(HANDLE hProcess, DWORD dwAddress, size_t nSize)
    {
        std::string strValue;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], nSize, 0);
        return strValue;
    }

    inline static std::vector<BYTE> ReadBytes(HANDLE hProcess, DWORD dwAddress, size_t nSize)
    {
        std::vector<BYTE> byValue;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &byValue[0], nSize, 0);
        return byValue;
    }

    inline static void WriteByte(HANDLE hProcess, DWORD dwAddress, DWORD dwValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, &dwValue, 1, 0);
    }

    inline static void Write4Byte(HANDLE hProcess, DWORD dwAddress, DWORD dwValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, &dwValue, 4, 0);
    }

    inline static void WriteFloat(HANDLE hProcess, DWORD dwAddress, float fValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, &fValue, 4, 0);
    }

    inline static void WriteString(HANDLE hProcess, DWORD dwAddress, std::string strValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], strValue.size(), NULL);
    }

    inline static void WriteBytes(HANDLE hProcess, DWORD dwAddress, std::vector<BYTE> byValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, byValue.data(), byValue.size(), NULL);
    }

    inline static DWORD GetDifference(DWORD dwSrcAddress, DWORD dwDestAddress)
    {
        DWORD dwDifference = 0;

        if (dwSrcAddress > dwDestAddress)
        {
            dwDifference = dwSrcAddress - dwDestAddress;

            if (dwDifference > 0)
                return (0xFFFFFFFB - dwDifference);
        }

        return (dwDestAddress - dwSrcAddress - 5);
    }

    inline static void ExecuteRemoteCode(HANDLE hProcess, BYTE* codes, size_t psize)
    {
        LPVOID pAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (pAddress == nullptr)
            return;

        WriteProcessMemory(hProcess, (LPVOID)pAddress, codes, psize, 0);
        HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pAddress, 0, 0, 0);

        if (hThread != nullptr)
        {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }

        VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);
    }
};

