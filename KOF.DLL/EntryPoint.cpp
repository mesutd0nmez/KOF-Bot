#include "pch.h"
#include "ReflectiveLoader.h"
#include <stdio.h>
#include "Bot.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT BOOL
EntryPoint(LPVOID lpParam, DWORD nParamLen)
{
	char* szParam = (char*)malloc(32 + nParamLen);

	if (szParam)
	{
		sprintf_s(szParam, 32 + nParamLen, "%s", (char*)lpParam);

#ifdef DEBUG
		printf("EntryPoint: %s\n", szParam);
#endif
		std::vector<std::string> vecParam = Tokenize(szParam, ' ');

		new std::thread([vecParam]()
		{
			Bot* bot = new Bot();

			bot->Initialize(
				(PlatformType)std::stoi(vecParam[0].c_str()),
				std::stoi(vecParam[1].c_str()));
		});

		free(szParam);
	}

    return TRUE;
}

#ifdef __cplusplus
}
#endif