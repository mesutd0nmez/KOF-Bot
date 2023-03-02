#include "pch.h"
#include "UI.h"
#include "Bot.h"
#include "ClientHandler.h"
#include "ReflectiveInjection.h"

#ifdef _WINDLL

extern HINSTANCE hAppInstance;
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved)
{
    BOOL bReturnValue = TRUE;

    switch (dwReason)
    {
        case DLL_QUERY_HMODULE:
            if (lpReserved != NULL)
                *(HMODULE*)lpReserved = hAppInstance;
            break;
        case DLL_PROCESS_ATTACH:
            hAppInstance = hinstDLL;
#ifdef DEBUG
            AllocConsole();
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif
            break;
        case DLL_PROCESS_DETACH:
        {
#ifdef DEBUG
            FreeConsole();
#endif
        }
        break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return bReturnValue;
}
#else

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif

    Bot* bot = new Bot();

    bot->Initialize();

    bool bWorking = true;

    while (bWorking)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (bot->GetInjectedProcessId() != 0 && bot->IsInjectedProcessLost())
            bWorking = false;

        bot->Process();
    }

#ifdef DEBUG
    FreeConsole();
#endif

    return 0;
}

#endif