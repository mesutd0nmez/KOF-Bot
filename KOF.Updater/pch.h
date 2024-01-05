// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include "../KOF.Bot/Define.h"

#include "SkCrypter.h"

#ifdef VMPROTECT
#include "VMProtectSDK.h"
#endif

#define Print(a, ...) printf("%s: " a "\n", __func__, ##__VA_ARGS__)

#endif