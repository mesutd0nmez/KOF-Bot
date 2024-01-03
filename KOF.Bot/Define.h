#pragma once

#define INJECT_PATH "C:\\NTTGame\\KnightOnlineEn"
#define INJECT_EXE "KnightOnLine.exe"
#define INJECT_PLATFORM 0

//#define GAME_PATH "C:\\CNKO"
//#define GAME_EXE "KnightOnLine.exe"
//#define GAME_PLATFORM 1

//#define GAME_PATH "C:\\Ongate\\KnightOnline"
//#define GAME_EXE "knightonline.exe"
//#define GAME_PLATFORM 2
//#define GAME_ACCOUNT_INDEX 1

//#define GAME_PATH "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Knight Online"
//#define GAME_EXE "KnightOnLine.exe"
//#define GAME_PLATFORM 3

#ifdef _DEBUG
#define DEBUG
#define DEBUG_LOG
//#define PRINT_PACKET
#else
#define ENABLE_TRIGGER_BSOD
#define ENABLE_ANTI_DEBUG
#define ENABLE_TRAP_CHECK
#define ENABLE_PONG_TIMEOUT_CHECK
#define ENABLE_SERVER_CONNECTION_LOST_CHECK
#define ENABLE_ANTI_SUSPEND_PROTECT
#define ENABLE_INTERACTIVE_CHECK
#define ENABLE_REPORT_WITH_SCREENSHOT
#endif

//#define ENABLE_MAIN_PERFORMANCE_COUNTER
//#define ENABLE_BOT_PERFORMANCE_COUNTER
//#define ENABLE_HANDLER_PERFORMANCE_COUNTER

#ifdef PRINT_PACKET
#define PRINT_RECV_PACKET
#define PRINT_SEND_PACKET
#endif

//#define DISABLE_XIGNCODE





