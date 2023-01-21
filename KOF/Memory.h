#pragma once
#include <thread>
#include <string>
#include <Windows.h>

class Memory
{
    public:
        __inline static BYTE ReadByte(BYTE address)
        {
            if (!IsBadReadPtr((VOID*)address, sizeof(BYTE)))
            {
                return(*(BYTE*)(address));
            }
            return 0;
        }

        __inline static void WriteByte(DWORD address, DWORD value)
        {
            WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &value, 1, 0);
        }

        __inline static DWORD Read4Byte(DWORD address)
        {
            if (!IsBadReadPtr((VOID*)address, sizeof(DWORD)))
            {
                return(*(DWORD*)(address));
            }
            return 0;
        }

        __inline static void Write4Byte(DWORD address, DWORD value)
        {
            WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &value, 4, 0);
        }

        __inline static float ReadFloat(BYTE address)
        {
            if (!IsBadReadPtr((VOID*)address, sizeof(float)))
            {
                return(*(float*)(address));
            }
            return 0.0f;
        }

        __inline static void WriteFloat(DWORD address, float value)
        {
            WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &value, 4, 0);
        }

        __inline static char* ReadString(DWORD address, long pSize)
        {
            char* strOutput = (char*)malloc(pSize + 1);
            ReadProcessMemory(GetCurrentProcess(), (LPVOID*)address, strOutput, pSize + 1, NULL);
            return strOutput;
        }

        __inline static void WriteString(DWORD address, std::string value)
        {
            WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, &value[0], value.size(), NULL);
        }
};

