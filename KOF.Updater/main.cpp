﻿#include "pch.h"

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
        } 
        while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return false;
}

bool StartProcess(std::string szFilePath, std::string szFile, std::string szCommandLine, PROCESS_INFORMATION& processInfo)
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
        return false;

    return true;
}

bool KillProcessesByFileName(const std::vector<const char*>& fileNames)
{
    DWORD currentProcessId = GetCurrentProcessId();

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
            if (pe.th32ProcessID == currentProcessId)
            {
                continue;
            }

            for (const char* fileName : fileNames)
            {
                if (_stricmp(pe.szExeFile, fileName) == 0)
                {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);

                    if (hProcess)
                    {
                        if (TerminateProcess(hProcess, 0))
                        {
#ifdef DEBUG_LOG
                            Print("KillProcessesByFileNames: Terminated process ID: %d", pe.th32ProcessID);
#endif
                        }
                        else
                        {
#ifdef DEBUG_LOG
                            Print("KillProcessesByFileNames: Failed to terminate process ID: %d", pe.th32ProcessID);
#endif
                        }

                        CloseHandle(hProcess);
                    }
                    else
                    {
#ifdef DEBUG_LOG
                        Print("KillProcessesByFileNames: Failed to open process for termination");
#endif
                    }
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    return true;
}

bool KillProcessesByFileName(const char* fileName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
#ifdef DEBUG_LOG
        Print("Error creating process snapshot");
#endif
        return false;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe))
    {
        do
        {
            if (_stricmp(pe.szExeFile, fileName) == 0)
            {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);

                if (hProcess)
                {
                    if (TerminateProcess(hProcess, 0))
                    {
#ifdef DEBUG_LOG
                        Print("Terminated process ID: %d", pe.th32ProcessID);
#endif
                    }
                    else
                    {
#ifdef DEBUG_LOG
                        Print("Failed to terminate process ID: %d", pe.th32ProcessID);
#endif
                    }

                    CloseHandle(hProcess);
                }
                else
                {
#ifdef DEBUG_LOG
                    Print("Bot: Failed to open process for termination");
#endif
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    return true;
}

bool Unzip(const char* zipFilePath, const char* destPath)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_file(&zip, zipFilePath, 0)) 
    {
#ifdef DEBUG_LOG
        Print("Unzip: Input file not found");
#endif
        return false;
    }

    _mkdir(destPath);

    int numFiles = mz_zip_reader_get_num_files(&zip);

    bool bExtracted = false;

    for (int i = 0; i < numFiles; ++i) 
    {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) 
        {
#ifdef DEBUG_LOG
            Print("Unzip: No file information");
#endif
            bExtracted = false;
            break;
        }

        char destFilePath[256];
        sprintf(destFilePath, "%s/%s", destPath, file_stat.m_filename);

        if (file_stat.m_is_directory) 
        {
            _mkdir(destFilePath);
        }
        else 
        {
            FILE* destFile = fopen(destFilePath, "wb");

            if (!destFile) 
            {
#ifdef DEBUG_LOG
                Print("Unzip: File create failed - %s", destFilePath);
#endif
                bExtracted = false;
                break;
            }

            void* buf = mz_zip_reader_extract_to_heap(&zip, i, (size_t*)&file_stat.m_uncomp_size, 0);

            if (buf) 
            {
                fwrite(buf, 1, (size_t)file_stat.m_uncomp_size, destFile);
                mz_free(buf);
            }

            bExtracted = true;

            fclose(destFile);
        }
    }
    
    mz_zip_reader_end(&zip);

    return bExtracted;
}

int main()
{
#ifdef DEBUG_LOG
    Print("Main: Update starting");
#endif

    // 1 - Check update file exist
    std::ifstream fUpdateFile(skCryptDec("Update.zip"));

    if (!fUpdateFile.is_open()) 
    {
#ifdef DEBUG_LOG
        Print("Main: Update file not found - %s", skCryptDec("Update.zip"));
#endif
        return 0;
    }

    // 2 - Kill all depended process
    std::vector<const char*> fileNames = { 
        skCryptDec("DSAServiceHelper.exe"),
        skCryptDec("KnightOnLine.exe"), 
        skCryptDec("xldr_KnightOnline_NA.exe"),
        skCryptDec("xldr_KnightOnline_NA_loader_win32.exe"),
        skCryptDec("xldr_KnightOnline_GB.exe"),
        skCryptDec("xldr_KnightOnline_GB_loader_win32.exe"),
        skCryptDec("xxd-0.xem.exe"),
    };

    KillProcessesByFileName(fileNames);

    while (IsProcessRunning(skCryptDec("DSAServiceHelper.exe")))
    {
        Sleep(1000);
        KillProcessesByFileName(skCryptDec("DSAServiceHelper.exe"));
        Sleep(1000);
    }

    // 3 - Start update process
    if(!Unzip(skCryptDec("Update.zip"), "."))
    {
#ifdef DEBUG_LOG
        Print("Main: Update file extraction failed");
#endif
        return 0;
    }

    Sleep(1000);

    // 4 - Re-Launch bot process
    PROCESS_INFORMATION botProcessInfo;
    if (!StartProcess(".", skCryptDec("DSAServiceHelper.exe"), "", botProcessInfo))
    {
#ifdef DEBUG_LOG
        Print("Main: Bot process start failed");
#endif
        return 0;
    }

    Sleep(1000);

    // 5 - Remove update file
    DeleteFileA(skCryptDec("Update.zip"));

#ifdef DEBUG_LOG
    Print("Main: Update completed");
#endif

    return 0;
}
