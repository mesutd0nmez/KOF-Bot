#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _WINSOCKAPI_

// Windows Header Files
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <ShlObj_core.h>
#include <vector>
#include <thread>
#include <map>
#include <set>
#include <shellapi.h>
#include <tlhelp32.h>
#include <mmsystem.h>
#include <winternl.h>
#include <chrono>
#include <iomanip>
#include <random>
#include <dwmapi.h>
#include <unordered_set>
#include <queue>
#include <Psapi.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <VersionHelpers.h>
#include <winsock2.h>
#include <Ws2ipdef.h>
#include <Ws2tcpip.h>
#include <functional>
#include <cerrno>
#include <d3d9.h>
#include <D3dx9tex.h>
#include <dbghelp.h>
#include <direct.h>