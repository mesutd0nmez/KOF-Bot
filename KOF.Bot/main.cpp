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
#ifdef DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    SetConsoleCtrlHandler(MyConsoleCtrlHandler, TRUE);
#endif

    std::ifstream updater2File("Updater2.exe");

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
}