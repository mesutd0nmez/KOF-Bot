#include "pch.h"
#include "UI.h"
#include "Drawing.h"
#include "Bot.h"
#include "ClientHandler.h"

Bot* bot = nullptr;

BOOL WINAPI MyConsoleCtrlHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_CLOSE_EVENT)
    {
        if (bot)
        {
            TerminateMyProcess(bot->GetInjectedProcessId(), -1);
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

#ifdef SET_DEBUG_CLIENT_INFORMATION
    std::string szClientPath = DEVELOPMENT_PATH;
    std::string szClientExe = DEVELOPMENT_EXE;
    PlatformType iPlatformType = (PlatformType)DEVELOPMENT_PLATFORM;
    int iAccountIndex = DEVELOPMENT_ACCOUNT_INDEX;
#else
    std::string szClientPath = "";
    std::string szClientExe = "";
    PlatformType iPlatformType = PlatformType::USKO;
    int iAccountIndex = -1;
#endif

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

    bot = new Bot();

    bot->Initialize(szClientPath, szClientExe, iPlatformType, iAccountIndex);

    while (true)
    {
        if (bot->IsClosed())
            break;

        if (Drawing::Done)
            break;

        if (bot->GetInjectedProcessId() != 0 && bot->IsInjectedProcessLost())
            break;

        bot->Process();
    }

    TerminateMyProcess(bot->GetInjectedProcessId(), -1);

#ifdef DEBUG
    fclose(stdout);
    FreeConsole();
#endif

    LocalFree(szArglist);

    return 0;
}