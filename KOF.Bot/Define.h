#pragma once

///////////////// DEBUG ////////////////////

#ifdef _DEBUG
#define DEBUG
#else
//#define DEBUG 
#endif

#ifdef DEBUG
#define PRINT_RECV_PACKET
#define PRINT_SEND_PACKET
#endif

#define DISABLE_XIGNCODE

///////////////// PROTECTION ////////////////////
//#define ENABLE_ATTACH_PROTECT
//#define ENABLE_SUSPEND_PROTECT
//#define ENABLE_SERVER_CONNECTION_LOST_PROTECT
//#define ENABLE_DEBUGGER_PRESENT_PROTECT
//#define ENABLE_PONG_TIMEOUT_PROTECT

///////////////// IMPORTANT ////////////////////

#define DEVELOPMENT_PATH "C:\\NTTGame\\KnightOnlineEn"
#define DEVELOPMENT_EXE "KnightOnLine.exe"
#define DEVELOPMENT_PLATFORM 0

///////////////// FEATURES /////////////////

//#define FEATURE_LEVEL_DOWNER
#define FEATURE_TOURNAMENT_SKILLS

//#define DEVELOPMENT_PATH "C:\\CNKO"
//#define DEVELOPMENT_EXE "KnightOnLine.exe"
//#define DEVELOPMENT_PLATFORM 1
//#define DEVELOPMENT_ACCOUNT_INDEX 9

//#define DEVELOPMENT_PATH "C:\\Ongate\\KnightOnline"
//#define DEVELOPMENT_EXE "knightonline.exe"
//#define DEVELOPMENT_PLATFORM 3
//#define DEVELOPMENT_ACCOUNT_INDEX 1

//#define DEVELOPMENT_PATH "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Knight Online"
//#define DEVELOPMENT_EXE "KnightOnLine.exe"
//#define DEVELOPMENT_PLATFORM 4
//#define DEVELOPMENT_ACCOUNT_INDEX 11

///////////////// UI ///////////////////////

//#define UI_HAZAR
//#define UI_BLANK
//#define UI_DEFAULT
//#define UI_COMMON
//#define DEVELOPER_ONLY



