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
#include <filesystem>
#include <sstream>
#include <ShlObj_core.h>
#include <vector>
#include <functional>
#include <thread>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <shellapi.h>
#include <limits>
#include <tlhelp32.h>
#include <cmath>
#include <mmsystem.h>
#include <locale>
#include <codecvt>
#include <charconv>
#include <winternl.h>
#include <future>
#include <chrono>
#include <iomanip>
#include <random>
#include <dwmapi.h>
