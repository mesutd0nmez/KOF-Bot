#pragma once

#define DEBUG
#define PRINT_RECV_PACKET
#define PRINT_SEND_PACKET

#define USE_MAILSLOT

//#define AUTO_LOGIN_TEST

//#define DISABLE_XIGNCODE

#ifdef DEBUG
#define DEVELOPMENT_PATH "C:\\NTTGame\\KnightOnlineEn"
#else
#define DEVELOPMENT_PATH ".\\"
#endif

#define MAX_ATTACK_RANGE		50.0f
#define MAX_VIEW_RANGE		100.0f