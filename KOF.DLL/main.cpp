#include "pch.h"
#include "UI.h"
#include "Bot.h"

#ifdef _WINDLL

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
#ifdef DEBUG
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif

        DisableThreadLibraryCalls(hinstDLL);

        auto player = std::make_unique<Bot>();
        player->Initialize();

        bool bWorking = true;

        while (bWorking)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (player->GetInjectedProcessId() != 0 && player->IsInjectedProcessLost())
                bWorking = false;
        }

#ifdef DEBUG
        FreeConsole();
#endif

        ExitThread(0);
    }

    return FALSE;
}

#else

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif

    auto player = std::make_unique<Bot>();

    player->Initialize();

    bool bWorking = true;

    while (bWorking)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (player->GetInjectedProcessId() != 0 && player->IsInjectedProcessLost())
            bWorking = false;
    }

#ifdef DEBUG
    FreeConsole();
#endif

    return 0;
}

#endif