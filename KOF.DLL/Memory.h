#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#define CopyBytes(Dest,Src) memcpy(Dest, (BYTE*)&Src, sizeof(Src))

class Memory
{
public:
    inline static BYTE ReadByte(DWORD dwAddress)
    {
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(BYTE)))
        {
            return(*(BYTE*)(dwAddress));
        }

        return 0;
    }

    inline static DWORD Read4Byte(DWORD dwAddress)
    {
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(DWORD)))
        {
            return(*(DWORD*)(dwAddress));
        }

        return 0;
    }

    inline static float ReadFloat(DWORD dwAddress)
    {
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(float)))
        {
            return(*(float*)(dwAddress));
        }

        return 0.0f;
    }

    inline static std::string ReadString(DWORD dwAddress, size_t nSize)
    {
        std::string strValue;
        ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &strValue[0], nSize, 0);
        return strValue;
    }

    inline std::vector<BYTE> ReadBytes(DWORD dwAddress, size_t nSize)
    {
        std::vector<BYTE> byValue;
        ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &byValue[0], nSize, 0);
        return byValue;
    }

    inline static void WriteByte(DWORD dwAddress, DWORD dwValue)
    {
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &dwValue, 1, 0);
    }

    inline static void Write4Byte(DWORD dwAddress, DWORD dwValue)
    {
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &dwValue, 4, 0);
    }

    inline static void WriteFloat(DWORD dwAddress, float fValue)
    {
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &fValue, 4, 0);
    }

    inline static void WriteString(DWORD dwAddress, std::string strValue)
    {
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, &strValue[0], strValue.size(), NULL);
    }

    inline static void WriteBytes(DWORD dwAddress, std::vector<BYTE> byValue)
    {
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddress, byValue.data(), byValue.size(), NULL);
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
};

