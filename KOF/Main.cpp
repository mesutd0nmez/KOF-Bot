#include "UI.h"
#include "Bot.h"
#include <stdio.h>
#include <thread>

#ifdef _WINDLL

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
#ifdef _DEBUG
            AllocConsole();
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif
            DisableThreadLibraryCalls(hinstDLL);

            new std::thread([]() { Bot::Process(); });
            new std::thread([]() { UI::Render(); });
        }
        break;
    }

    return TRUE;
}

#else

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    UI::Render();

    return 0;
}

#endif