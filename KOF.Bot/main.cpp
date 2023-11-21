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

    std::string szClientPath = DEVELOPMENT_PATH;
    std::string szClientExe = DEVELOPMENT_EXE;
    PlatformType iPlatformType = (PlatformType)DEVELOPMENT_PLATFORM;
    int iAccountIndex = DEVELOPMENT_ACCOUNT_INDEX;

    int argc;

    LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);

    char** argv = new char* [argc];

    for (int i = 0; i < argc; i++)
    {
        int lgth = wcslen(szArglist[i]);

        argv[i] = new char[lgth + 1];

        for (int j = 0; j <= lgth; j++)
            argv[i][j] = char(szArglist[i][j]);
    }

    if (argc == 5)
    {
        szClientPath = to_string(szArglist[1]);
        szClientExe = to_string(szArglist[2]);
        iPlatformType = (PlatformType)std::stoi(szArglist[3]);
        iAccountIndex = std::stoi(szArglist[4]);
    }

#ifdef DEBUG
        printf("Bot: All client process forcing close\n");
#endif

        KillProcessesByFileName(skCryptDec("KnightOnLine.exe"));

        if (iPlatformType == PlatformType::USKO) 
        {
            KillProcessesByFileName(skCryptDec("xldr_KnightOnline_NA.exe"));
            KillProcessesByFileName(skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"));
        }
        
        if (iPlatformType == PlatformType::STKO)
        {
            KillProcessesByFileName(skCryptDec("xldr_KnightOnline_GB.exe"));
            KillProcessesByFileName(skCryptDec("xldr_KnightOnline_GB_loader_win32.exe"));
        }

        if (iPlatformType == PlatformType::USKO
            || iPlatformType == PlatformType::STKO
            || iPlatformType == PlatformType::KOKO)
        {
            KillProcessesByFileName(skCryptDec("xxd-0.xem"));
        }

    m_Bot = new Bot();
    m_Bot->Initialize(szClientPath, szClientExe, iPlatformType, iAccountIndex);

    UI::Render(m_Bot);

    TerminateMyProcess(m_Bot->GetInjectedProcessId(), -1);

#ifdef DEBUG
    fclose(stdout);
    FreeConsole();
#endif

    LocalFree(szArglist);

    return 0;
}