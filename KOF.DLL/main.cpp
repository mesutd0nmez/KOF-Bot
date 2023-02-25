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

        new std::thread([]()
        {
            auto player = std::make_unique<Bot>();
            player->Initialize();
        });

        return TRUE;
    }

    return TRUE;
}

#else

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif

    Bot bot;
    bot.Initialize();

    return 0;
}

#endif