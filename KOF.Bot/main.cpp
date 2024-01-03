#include "pch.h"
#include "UI.h"
#include "Bot.h"
#include "ClientHandler.h"

Bot* m_Bot = nullptr;

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
#ifdef VMPROTECT
    VMProtectBeginMutation("ConsoleCtrlHandler");
#endif

    switch (dwCtrlType)
    {
#ifndef DEBUG_LOG
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        {
#if !defined(ENABLE_MAIN_PERFORMANCE_COUNTER) && !defined(ENABLE_BOT_PERFORMANCE_COUNTER) && !defined(ENABLE_HANDLER_PERFORMANCE_COUNTER)
            if (m_Bot)
            {
                m_Bot->SendReport(
                    dwCtrlType == CTRL_C_EVENT 
                    ? REPORT_CODE_DETECT_CONSOLE_CTRL_C_EVENT 
                    : REPORT_CODE_DETECT_CONSOLE_BREAK_EVENT);

#ifdef ENABLE_REPORT_WITH_SCREENSHOT
                std::vector<uint8_t> vecImageBuffer = CaptureScreen(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
                m_Bot->SendScreenshot(vecImageBuffer);

#endif
            }

#ifdef ENABLE_TRIGGER_BSOD
            BOOLEAN bEnabled;
            ULONG uResponse;
            RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled);
            NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, &uResponse);
#endif
            exit(0);
#endif
        }
        break;
#endif
        case CTRL_CLOSE_EVENT:
        {
            if (m_Bot)
            {
                TerminateProcess(m_Bot->GetInjectedProcessHandle(), 0);
            }

            exit(0);
        }
        break;
    }

    return TRUE;

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef VMPROTECT
    VMProtectBeginUltra("wWinMain");
#endif

#if defined(DEBUG_LOG) || defined(ENABLE_MAIN_PERFORMANCE_COUNTER) || defined(ENABLE_BOT_PERFORMANCE_COUNTER) || defined(ENABLE_HANDLER_PERFORMANCE_COUNTER)
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif

    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    if (!IsUserAnAdmin()) 
    {
#ifdef DEBUG_LOG
        Print("Application is not running on Administrator privileges");
#endif
        return 0;
    }

    HardwareInformation* pHardwareInfo = new HardwareInformation();
    pHardwareInfo->LoadHardwareInformation();

    DeleteFilesInPrefetchFolder();

    m_Bot = new Bot(pHardwareInfo);

#if defined(DEBUG_LOG) || defined(ENABLE_MAIN_PERFORMANCE_COUNTER) || defined(ENABLE_BOT_PERFORMANCE_COUNTER) || defined(ENABLE_HANDLER_PERFORMANCE_COUNTER)
    fclose(stdout);
    FreeConsole();
#endif

    return 0;

#ifdef VMPROTECT
    VMProtectEnd();
#endif
}