#pragma once

//winapi shit
#include <Windows.h>

//enum shit
#include <TlHelp32.h>
#include <Psapi.h>

//string shit
#include <strsafe.h>
#include <tchar.h>

//file shit
#include <fstream>

//dank shit
#include <vector>
#include <ctime>

//session shit
#include <wtsapi32.h>


#pragma warning(disable: 6001) //uninitialized memory (bug with SAL notation)
#pragma warning(disable: 6031) //ignored return value warning
#pragma warning(disable: 6258) //TerminateThread warning