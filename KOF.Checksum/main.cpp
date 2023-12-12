#include "pch.h"

DWORD CalculateCRC32(const std::string& filePath)
{
    std::ifstream fileStream(filePath, std::ios::binary);

    if (!fileStream.is_open())
    {
#ifdef DEBUG
        printf("File not opened: %s", filePath.c_str());
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

int main()
{
    LPWSTR* argv;
    int argc;
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc > 1) 
    {
        std::wstring wszfileName = argv[1];
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string szFileName = converter.to_bytes(wszfileName);

        DWORD iCRC = CalculateCRC32(szFileName.c_str());

        printf("CRC: %u\n", iCRC);

        LocalFree(argv);
    }

    while (true)
    {
        Sleep(1000);
    }

    return 0;
}
