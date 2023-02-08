#include "pch.h"
#include "UI.h"
#include "Bootstrap.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
//#ifdef _DEBUG
            AllocConsole();
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
//#endif
            DisableThreadLibraryCalls(hModule);

            printf("EntryPoint::Starting Threads\n");

            new std::thread([]()
            {
                printf("EntryPoint::Bootstrap\n");

                Bootstrap bootstrap;
                bootstrap.Start();

                printf("EntryPoint::UI\n");

                UI ui;
                ui.Render();
            });
        }
        break;

        case DLL_THREAD_ATTACH:
        {
            printf("EntryPoint::DLL_THREAD_ATTACH\n");
        }
        break;

        case DLL_THREAD_DETACH:
        {
            printf("EntryPoint::DLL_THREAD_DETACH\n");
        }
        break;

        case DLL_PROCESS_DETACH:
        {
            printf("EntryPoint::DLL_PROCESS_DETACH\n");
        }
        break;
    }

    return TRUE;
}

