#include "pch.h"
#include "UI.h"
#include "Bot.h"
#include "ClientHandler.h"

Bot* m_Bot = nullptr;

BOOL WINAPI MyConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_CLOSE_EVENT)
    {
        if (m_Bot)
        {
            TerminateMyProcess(m_Bot->GetInjectedProcessId(), -1);
        }
    }

    return TRUE;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef VMPROTECT
    VMProtectBeginUltra("wWinMain");
#endif

#ifdef ENABLE_ATTACH_PROTECT
    HMODULE hMod = GetModuleHandleW(skCryptDec(L"ntdll.dll"));

    if (!hMod)
        return 0;

    PATCH_FUNC patchFunctions[] =
    {
        { skCryptDec("DbgBreakPoint"), 0, DbgBreakPoint_FUNC_SIZE },
        { skCryptDec("DbgUiRemoteBreakin"), 0,DbgUiRemoteBreakin_FUNC_SIZE },
        { skCryptDec("NtContinue"), 0, NtContinue_FUNC_SIZE }
    };
    
    for (int i = 0; i < _countof(patchFunctions); i++)
    {
        FARPROC funcAddr = GetProcAddress(hMod, patchFunctions[i].funcName.c_str());

        if (funcAddr)
            patchFunctions[i].funcAddr = funcAddr;
        else
            patchFunctions[i].funcAddr = nullptr;
    }

    WCHAR szModName[MAX_PATH] = { 0 };
    HANDLE hProcess = GetCurrentProcess();

    if (!GetModuleBaseNameW(hProcess, hMod, szModName, _countof(szModName)))
        return 0;

    if (!wcsstr(szModName, skCryptDec(L"ntdll")) && !wcsstr(szModName, skCryptDec(L"NTDLL")))
        return 0;

    for (int i = 0; i < _countof(patchFunctions); i++)
    {
        if (patchFunctions[i].funcAddr == nullptr)
            return 0;

        WriteProcessMemory(hProcess, patchFunctions[i].funcAddr, patchFunctions[i].funcAddr, patchFunctions[i].funcSize, 0);
    }
#endif

#ifdef DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    SetConsoleCtrlHandler(MyConsoleCtrlHandler, TRUE);
#endif

    std::ifstream updater2File(skCryptDec("Updater2.exe"));

    if (updater2File)
    {
        updater2File.close();

        Sleep(1000);
        DeleteFileA(skCryptDec("Updater.exe"));
        Sleep(1000);
        MoveFileA(skCryptDec("Updater2.exe"), skCryptDec("Updater.exe"));
        Sleep(1000);
    }

    m_Bot = new Bot();

    TerminateMyProcess(m_Bot->GetInjectedProcessId(), -1);

#ifdef DEBUG
    fclose(stdout);
    FreeConsole();
#endif

    return 0;

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}