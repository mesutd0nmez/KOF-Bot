#pragma once

#include "pch.h"

namespace VMProtect
{
	static float m_fiLastCheckValidImageCRCTime = TimeGet();
	static float m_fiLastCheckIsDebuggerPresentTime = TimeGet();

	static std::string m_szPayload = "";
	inline static std::string GetLastPayload() { return m_szPayload; }

	inline static ReportCode IsDetected()
	{
#ifdef VMPROTECT
		VMProtectBeginMutation("VMProtect::IsDetected");
#endif
		float fCurrentTime = TimeGet();

		if (fCurrentTime > (m_fiLastCheckValidImageCRCTime + (2064.0f / 1000.0f)))
		{
#ifdef VMPROTECT
			if (!VMProtectIsValidImageCRC())
			{
				return REPORT_CODE_DETECT_VMPROTECT_IS_VALID_IMAGE_CRC;
			}
#endif
			m_fiLastCheckValidImageCRCTime = TimeGet();
		}

		if (fCurrentTime > (m_fiLastCheckIsDebuggerPresentTime + (5464.0f / 1000.0f)))
		{
#ifdef VMPROTECT
			if (VMProtectIsDebuggerPresent(true))
			{
				return REPORT_CODE_DETECT_VMPROTECT_IS_DEBUGGER_PRESENT;
			}
#endif
			m_fiLastCheckIsDebuggerPresentTime = TimeGet();
		}

		return REPORT_CODE_NONE;

#ifdef VMPROTECT
		VMProtectEnd();
#endif
	}
}