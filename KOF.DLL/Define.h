#pragma once

#define DEBUG
//#define PRINT_RECV_PACKET
//#define PRINT_SEND_PACKET

#ifdef DEBUG
#define DEVELOPMENT_PATH "C:\\CNKO"
#else
#define DEVELOPMENT_PATH ".\\"
#endif

#define MAX_ATTACK_RANGE		50.0f
#define MAX_VIEW_RANGE		100.0f