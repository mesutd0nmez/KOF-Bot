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

#ifdef DEBUG
    std::string szClientPath = DEVELOPMENT_PATH;
    std::string szClientExe = DEVELOPMENT_EXE;
    PlatformType iPlatformType = (PlatformType)DEVELOPMENT_PLATFORM;
    int iAccountIndex = DEVELOPMENT_ACCOUNT_INDEX;
    bool bForceCloseClient = false;
#else
    std::string szClientPath = "";
    std::string szClientExe = "";
    PlatformType iPlatformType = PlatformType::USKO;
    int iAccountIndex = -1;
    bool bForceCloseClient = false;
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

    if (argc == 6)
    {
        bForceCloseClient = std::stoi(szArglist[5]) == 1 ? true : false;
    }

    //TODO: Implement later
    bForceCloseClient = true;

    if (bForceCloseClient)
    {
#ifdef DEBUG
        printf("Bot: All client process forcing close\n");
#endif

        KillProcessesByFileName("KnightOnLine.exe");

        if (iPlatformType == PlatformType::USKO) 
        {
            KillProcessesByFileName("xldr_KnightOnline_NA.exe");
            KillProcessesByFileName("xldr_KnightOnline_NA_loader_win32.exe");
        }
        
        if (iPlatformType == PlatformType::STKO)
        {
            KillProcessesByFileName("xldr_KnightOnline_GB.exe");
            KillProcessesByFileName("xldr_KnightOnline_GB_loader_win32.exe");
        }

        if (iPlatformType == PlatformType::USKO
            || iPlatformType == PlatformType::STKO
            || iPlatformType == PlatformType::KOKO)
        {
            KillProcessesByFileName("xxd-0.xem");
        }
    }

    m_Bot = new Bot();
    m_Bot->Initialize(szClientPath, szClientExe, iPlatformType, iAccountIndex);

    UI::Initialize(m_Bot);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (m_Bot->IsClosed())
            break;

        if (Drawing::Done)
            break;

        if (m_Bot->GetInjectedProcessId() != 0 && m_Bot->IsInjectedProcessLost())
            break;

        m_Bot->Process();

        if (m_Bot->GetUserConfiguration() != nullptr) 
        {
            UI::Render();
        }
    }

    TerminateMyProcess(m_Bot->GetInjectedProcessId(), -1);

    UI::Clear();

#ifdef DEBUG
    fclose(stdout);
    FreeConsole();
#endif

    LocalFree(szArglist);

    return 0;
}