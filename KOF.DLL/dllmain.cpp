#include "pch.h"
#include "UI.h"
#include "Bot.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
#ifdef _DEBUG
            AllocConsole();
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif
            DisableThreadLibraryCalls(hModule);

            new std::thread([]()
            {
                Bot bot;
                bot.Start();
            });

            new std::thread([]()
            {
                UI ui;
                ui.Render();
            });
        }
        break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

