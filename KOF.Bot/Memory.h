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
        BYTE nValue = 0;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 1, 0);
        return nValue;
    }

    inline static DWORD Read4Byte(HANDLE hProcess, DWORD dwAddress)
    {
        DWORD nValue = 0;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 4, 0);
        return nValue;
    }

    inline static float ReadFloat(HANDLE hProcess, DWORD dwAddress)
    {
        float nValue = 0.0f;
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 4, 0);

        return nValue;
    }

    inline static std::string ReadString(HANDLE hProcess, DWORD dwAddress, size_t nSize)
    {
        std::string strValue = "";
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], nSize, 0);
        return strValue;
    }

    inline static std::vector<BYTE> ReadBytes(HANDLE hProcess, DWORD dwAddress, size_t nSize)
    {
        std::vector<BYTE> byValue(nSize);
        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &byValue[0], nSize, 0);
        return byValue;
    }

    inline static void WriteByte(HANDLE hProcess, DWORD dwAddress, BYTE byValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, &byValue, 1, 0);
    }

    inline static void Write4Byte(HANDLE hProcess, DWORD dwAddress, int iValue)
    {
        WriteProcessMemory(hProcess, (LPVOID)dwAddress, &iValue, 4, 0);
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

    inline static void ExecuteRemoteCode(HANDLE hProcess, BYTE* byCode, size_t bySize)
    {
        LPVOID pAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (pAddress == nullptr)
        {
            return;
        }

        WriteProcessMemory(hProcess, pAddress, byCode, bySize, NULL);
        ExecuteRemoteCode(hProcess, pAddress);
        VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);
    }

    inline static void ExecuteRemoteCode(HANDLE hProcess, LPVOID pAddress)
    {
        HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pAddress, 0, 0, 0);

        if (hThread != nullptr)
        {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
    }
};

