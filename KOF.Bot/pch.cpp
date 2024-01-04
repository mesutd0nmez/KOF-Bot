// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

BOOL StartProcess(std::string szFilePath, std::string szFile, std::string szCommandLine, PROCESS_INFORMATION& processInfo)
{
	STARTUPINFO info = { sizeof(info) };

	std::string szWorkingDirectory(szFilePath.begin(), szFilePath.end());
	std::string strFileName(szFile.begin(), szFile.end());

	std::ostringstream fileArgs;
	fileArgs << szWorkingDirectory.c_str() << skCryptDec("\\") << strFileName.c_str();

	std::string file = fileArgs.str();

	std::ostringstream cmdArgs;
	cmdArgs << skCryptDec("\"") << szWorkingDirectory.c_str() << skCryptDec("\\") << strFileName.c_str() << skCryptDec("\"");
	cmdArgs << " ";
	cmdArgs << szCommandLine.c_str();

	std::string cmd = cmdArgs.str();

	SECURITY_ATTRIBUTES securityInfo = { sizeof(securityInfo) };

	securityInfo.bInheritHandle = FALSE;

	BOOL result = CreateProcess(&file[0], &cmd[0], &securityInfo, NULL, FALSE, 0, NULL, &szWorkingDirectory[0], &info, &processInfo);

	if (!result)
		return FALSE;

	return TRUE;
}

bool KillProcessesByFileName(const char* fileName) 
{
	std::vector<const char*> vecFileName { fileName };
	return KillProcessesByFileName(vecFileName);
}

bool KillProcessesByFileName(const std::vector<const char*>& fileNames)
{
	DWORD dwCurrentProcessId = GetCurrentProcessId();

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
#ifdef DEBUG_LOG
		Print("KillProcessesByFileNames: Error creating process snapshot");
#endif
		return false;
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe))
	{
		do
		{
			if (pe.th32ProcessID == dwCurrentProcessId)
				continue;

			for (const char* fileName : fileNames)
			{
				if (_stricmp(pe.szExeFile, fileName) != 0)
					continue;

				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);

				if (!hProcess)
					continue;

				if (!TerminateProcess(hProcess, 1))
				{
					CloseHandle(hProcess);
					continue;
				}

				CloseHandle(hProcess);
			}
		} while (Process32Next(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);

	return true;
}

uint8_t HexCharToUint8(char c) 
{
	if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
	if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
	if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
	throw std::invalid_argument(skCryptDec("Invalid hex character"));
}

std::vector<uint8_t> FromHexString(const std::string& hexString) 
{
	if (hexString.empty())
		return {};

	if (hexString.size() % 2 != 0)
		throw std::invalid_argument(skCryptDec("Bad hex length"));

	std::vector<uint8_t> result(hexString.size() / 2);

	for (size_t i = 0; i < hexString.size(); i += 2) 
	{
		result[i / 2] = (HexCharToUint8(hexString[i]) << 4) | HexCharToUint8(hexString[i + 1]);
	}

	return result;
}

std::string ToHexString(const std::vector<uint8_t>& bytes) 
{
	if (bytes.size() == 0)
		return "";

	std::stringstream ss;
	for (auto c : bytes)
		ss << std::setw(2) << std::setfill('0') << std::hex << (int)c;

	return ss.str();
}

DWORD CalculateCRC32(const std::string& filePath)
{
	std::ifstream fileStream(filePath, std::ios::binary);

	if (!fileStream.is_open())
	{
#ifdef DEBUG_LOG
		Print("File not opened: %s\n", filePath.c_str());
#endif
		return 0xFFFFFFFF;
	}

	fileStream.seekg(0, std::ios::end);
	uint32_t iFileSize = (uint32_t)fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);

	std::vector<char> buffer(iFileSize);

	fileStream.read(buffer.data(), iFileSize);
	fileStream.close();

	return crc32((uint8_t*)buffer.data(), iFileSize);
}

bool IsProcessRunning(const char* fileName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &pe32))
	{
		do
		{
			if (_stricmp(pe32.szExeFile, fileName) == 0)
			{
				CloseHandle(hSnap);
				return true;
			}
		} while (Process32Next(hSnap, &pe32));
	}

	CloseHandle(hSnap);
	return false;
}

std::string GenerateAlphanumericString(int length) 
{
	const std::string characters = skCryptDec("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	std::string alphanumericString = "";
	int charactersLength = characters.length();

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	for (int i = 0; i < length; ++i) 
	{
		int randomIndex = std::rand() % charactersLength;
		alphanumericString += characters[randomIndex];
	}

	return alphanumericString;
}

std::string to_string(wchar_t const* wcstr)
{
	auto s = std::mbstate_t();
	auto const target_char_count = std::wcsrtombs(nullptr, &wcstr, 0, &s);
	if (target_char_count == static_cast<std::size_t>(-1))
	{
		throw std::logic_error(skCryptDec("Illegal byte sequence"));
	}

	// +1 because std::string adds a null terminator which isn't part of size
	auto str = std::string(target_char_count, '\0');
	std::wcsrtombs(const_cast<char*>(str.data()), &wcstr, str.size() + 1, &s);
	return str;
}

std::string RemainingTime(long long int seconds) 
{
	const int secondsPerMinute = 60;
	const int minutesPerHour = 60;
	const int hoursPerDay = 24;

	long long int remainingDays = seconds / (secondsPerMinute * minutesPerHour * hoursPerDay);
	seconds %= secondsPerMinute * minutesPerHour * hoursPerDay;

	long long int remainingHours = seconds / (secondsPerMinute * minutesPerHour);
	seconds %= secondsPerMinute * minutesPerHour;

	long long int remainingMinutes = seconds / secondsPerMinute;
	seconds %= secondsPerMinute;

	std::string result;

	if (remainingDays > 0) 
	{
		result += std::to_string(remainingDays) + skCryptDec(" gun ");
	}

	if (remainingHours > 0) 
	{
		result += std::to_string(remainingHours) + skCryptDec(" saat ");
	}

	if (remainingMinutes > 0) 
	{
		result += std::to_string(remainingMinutes) + skCryptDec(" dakika ");
	}

	if (seconds > 0 || result.empty()) 
	{
		result += std::to_string(seconds) + skCryptDec(" saniye ");
	}

	if (remainingDays == 0 && remainingHours == 0 && remainingMinutes == 0 && seconds == 0) 
	{
		result += skCryptDec("0 saniye ");
	}

	return result;
}

float TimeGet()
{
#ifdef VMPROTECT
	VMProtectBeginMutation("TimeGet");
#endif

	static BOOL bInit = FALSE;
	static BOOL bUseHWTimer = FALSE;
	static LARGE_INTEGER nTime, nFrequency;

	if (bInit == FALSE)
	{
		if (TRUE == ::QueryPerformanceCounter(&nTime))
		{
			::QueryPerformanceFrequency(&nFrequency);
			bUseHWTimer = TRUE;
		}
		else
		{
			bUseHWTimer = FALSE;
		}

		bInit = TRUE;
	}

	if (bUseHWTimer)
	{
		::QueryPerformanceCounter(&nTime);
		return (float)((double)(nTime.QuadPart) / (double)nFrequency.QuadPart);
	}

	return (float)timeGetTime() / 1000.0f;

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

std::vector<uint8_t> CaptureScreen(int width, int height, int x, int y)
{
#ifdef VMPROTECT
	VMProtectBeginMutation("CaptureScreen");
#endif

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
	SelectObject(hdcMem, hBitmap);

	BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

	BITMAPFILEHEADER bmpFileHeader;
	bmpFileHeader.bfType = 0x4D42; // "BM"
	bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 4;
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER bmpInfoHeader;
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biWidth = width;
	bmpInfoHeader.biHeight = -height;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 32;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biSizeImage = 0;
	bmpInfoHeader.biXPelsPerMeter = 0;
	bmpInfoHeader.biYPelsPerMeter = 0;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biClrImportant = 0;

	std::vector<uint8_t> vecImageData(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 4);
	memcpy(vecImageData.data(), &bmpFileHeader, sizeof(BITMAPFILEHEADER));
	memcpy(vecImageData.data() + sizeof(BITMAPFILEHEADER), &bmpInfoHeader, sizeof(BITMAPINFOHEADER));
	GetDIBits(hdcScreen, hBitmap, 0, height, vecImageData.data() + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), (BITMAPINFO*)&bmpInfoHeader, DIB_RGB_COLORS);

	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);

	return vecImageData;

#ifdef VMPROTECT
	VMProtectEnd();
#endif
}

void DeleteFilesInPrefetchFolder() 
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(_T(skCryptDec("C:\\Windows\\Prefetch\\*")), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
		return;

	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
		{
			TCHAR filePath[MAX_PATH];
			_stprintf_s(filePath, _T(skCryptDec("C:\\Windows\\Prefetch\\%s")), findFileData.cFileName);
			DeleteFile(filePath);
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}