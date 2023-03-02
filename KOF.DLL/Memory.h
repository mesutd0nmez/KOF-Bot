#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#define CopyBytes(Dest,Src) memcpy(Dest, (BYTE*)&Src, sizeof(Src))

class Memory
{
public:
    inline static BYTE ReadByte(DWORD iProcessID, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(BYTE)))
        {
            return(*(BYTE*)(dwAddress));
        }

        return 0;
#else
        BYTE byValue = 0;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            ReadProcessMemory(hProcess, (LPVOID)dwAddress, &byValue, sizeof(BYTE), 0);
            CloseHandle(hProcess);
        }
       
        return byValue;
#endif
    }

    inline static DWORD Read4Byte(DWORD iProcessID, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(DWORD)))
        {
            return(*(DWORD*)(dwAddress));
        }

        return 0;
#else
        DWORD dwValue = 0;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            ReadProcessMemory(hProcess, (LPVOID)dwAddress, &dwValue, sizeof(DWORD), 0);
            CloseHandle(hProcess);
        }
     
        return dwValue;
#endif
    }

    inline static float ReadFloat(DWORD iProcessID, DWORD dwAddress)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, sizeof(float)))
        {
            return(*(float*)(dwAddress));
        }

        return 0;
#else
        float fValue = 0.0f;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            ReadProcessMemory(hProcess, (LPVOID)dwAddress, &fValue, sizeof(float), 0);
            CloseHandle(hProcess);
        }
        
        return fValue;
#endif
    }

    inline static std::string ReadString(DWORD iProcessID, DWORD dwAddress, size_t nSize)
    {
#ifdef _WINDLL
        if (!IsBadReadPtr((LPVOID*)dwAddress, nSize))
        {
            return(*(std::string*)(dwAddress));
        }

        return "";
#else
        std::string strValue;

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            ReadProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], nSize, 0);
            CloseHandle(hProcess);
        }
       
        return strValue;
#endif
    }

    inline static std::vector<BYTE> ReadBytes(DWORD iProcessID, DWORD dwAddress, size_t nSize)
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
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            ReadProcessMemory(hProcess, (LPVOID)dwAddress, &byValue[0], nSize, 0);
            CloseHandle(hProcess);
        }

        return byValue;
#endif
    }

    inline static void WriteByte(DWORD iProcessID, DWORD dwAddress, BYTE byValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, sizeof(BYTE)))
        {
            memcpy((LPVOID*)dwAddress, &byValue, sizeof(BYTE));
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &byValue, 1, 0);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void Write4Byte(DWORD iProcessID, DWORD dwAddress, DWORD dwValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, sizeof(DWORD)))
        {
            memcpy((LPVOID*)dwAddress, &dwValue, sizeof(DWORD));
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &dwValue, 4, 0);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void WriteFloat(DWORD iProcessID, DWORD dwAddress, float fValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, sizeof(float)))
        {
            memcpy((LPVOID*)dwAddress, &fValue, sizeof(float));
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &fValue, 4, 0);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void WriteString(DWORD iProcessID, DWORD dwAddress, std::string strValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, strValue.size()))
        {
            memcpy((LPVOID*)dwAddress, &strValue[0], strValue.size());
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            WriteProcessMemory(hProcess, (LPVOID)dwAddress, &strValue[0], strValue.size(), NULL);
            CloseHandle(hProcess);
        }
#endif
    }

    inline static void WriteBytes(DWORD iProcessID, DWORD dwAddress, std::vector<BYTE> byValue)
    {
#ifdef _WINDLL
        if (!IsBadWritePtr((LPVOID*)dwAddress, byValue.size()))
        {
            memcpy((LPVOID*)dwAddress, byValue.data(), byValue.size());
        }
#else
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
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

    inline static void ExecuteRemoteCode(DWORD iProcessID, BYTE* byCode, size_t bySize)
    {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID);

        if (hProcess != 0)
        {
            LPVOID pAddress = VirtualAllocEx(hProcess, 0, 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

            if (pAddress == nullptr)
                return;

#ifdef _WINDLL
            if (!IsBadWritePtr((LPVOID*)pAddress, bySize))
            {
                memcpy((LPVOID*)pAddress, byCode, bySize);
            }
#else
            WriteProcessMemory(hProcess, (LPVOID)pAddress, byCode, bySize, NULL);
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
    }
};

