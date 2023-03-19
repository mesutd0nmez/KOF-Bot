#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#define CopyBytes(Dest,Src) memcpy(Dest, (BYTE*)&Src, sizeof(Src))

class Memory
{
public:
    inline static BYTE ReadByte(DWORD dwProcessId, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(BYTE)))
        {
            return(*(BYTE*)(dwAddress));
        }

        return 0;
#else
        BYTE nValue = 0;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess == nullptr)
            return nValue;

        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 1, 0);
        CloseHandle(hProcess);

        return nValue;
#endif
    }

    inline static DWORD Read4Byte(DWORD dwProcessId, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(DWORD)))
        {
            return(*(DWORD*)(dwAddress));
        }

        return 0;
#else
        DWORD nValue = 0;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess == nullptr)
            return nValue;

        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 4, 0);
        CloseHandle(hProcess);

        return nValue;
#endif
    }

    inline static float ReadFloat(DWORD dwProcessId, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(float)))
        {
            return(*(float*)(dwAddress));
        }

        return 0;
#else
        float nValue = 0.0f;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess == nullptr)
            return nValue;

        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &nValue, 4, 0);
        CloseHandle(hProcess);

        return nValue;
#endif
    }

    inline static std::string ReadString(DWORD dwProcessId, DWORD dwAddress, size_t nSize)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, nSize))
        {
            return(*(std::string*)(dwAddress));
        }

        return "";
#else
        std::string strValue = "";

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess == nullptr)
            return strValue;

        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], nSize, 0);
        CloseHandle(hProcess);

        return strValue;
#endif
    }

    inline static std::vector<BYTE> ReadBytes(DWORD dwProcessId, DWORD dwAddress, size_t nSize)
    {
        std::vector<BYTE> byValue(nSize);

#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, nSize))
        {
            BYTE readBytes = *(BYTE*)(dwAddress);
            memcpy(byValue.data(), &readBytes, nSize);
        }

        return byValue;
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess == nullptr)
            return byValue;

        ReadProcessMemory(hProcess, (LPVOID)dwAddress, &byValue[0], nSize, 0);
        CloseHandle(hProcess);

        return byValue;
#endif
    }

    inline static void WriteByte(DWORD dwProcessId, DWORD dwAddress, BYTE byValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, sizeof(BYTE)))
        {
            memcpy((LPVOID*)dwAddress, &byValue, sizeof(BYTE));
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess != nullptr)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &byValue, 1, 0);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void Write4Byte(DWORD dwProcessId, DWORD dwAddress, DWORD dwValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, sizeof(DWORD)))
        {
            memcpy((LPVOID*)dwAddress, &dwValue, sizeof(DWORD));
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess != nullptr)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &dwValue, 4, 0);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void WriteFloat(DWORD dwProcessId, DWORD dwAddress, float fValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, sizeof(float)))
        {
            memcpy((LPVOID*)dwAddress, &fValue, sizeof(float));
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess != nullptr)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &fValue, 4, 0);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void WriteString(DWORD dwProcessId, DWORD dwAddress, std::string strValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, strValue.size()))
        {
            memcpy((LPVOID*)dwAddress, &strValue[0], strValue.size());
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess != nullptr)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], strValue.size(), NULL);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void WriteBytes(DWORD dwProcessId, DWORD dwAddress, std::vector<BYTE> byValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, byValue.size()))
        {
            memcpy((LPVOID*)dwAddress, byValue.data(), byValue.size());
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess != nullptr)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, byValue.data(), byValue.size(), NULL);
            CloseHandle(hProcess);
        }
#endif
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

    inline static void ExecuteRemoteCode(DWORD dwProcessId, BYTE* byCode, size_t bySize)
    {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

        if (hProcess == nullptr)
            return;

        LPVOID pAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (pAddress == nullptr)
        {
            CloseHandle(hProcess);
            return;
        }

#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)pAddress, bySize))
        {
            memcpy((LPVOID*)pAddress, byCode, bySize);
        }
#else
        WriteProcessMemory(hProcess, pAddress, byCode, bySize, NULL);
#endif

        HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pAddress, 0, 0, 0);

        if (hThread != nullptr)
        {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }

        VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE);

        CloseHandle(hProcess);
    }
};

