#pragma once

#include "pch.h"

namespace Trap
{
	// =======================================================================
	// Close Handle Exception Trap
	// =======================================================================

	inline static BOOL CheckCloseHandleExceptionTrap()
	{
		HANDLE hInvalid = (HANDLE)0xBEEF; // an invalid handle
		DWORD bIsTrapped = FALSE;

		__try
		{
			CloseHandle(hInvalid);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bIsTrapped = TRUE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// Prefix Hop Trap
	// =======================================================================

	inline static BOOL CheckPrefixHopTrap()
	{
		BOOL bIsTrapped = TRUE;

		__try
		{
			_asm
			{
				__emit 0xF3;	// 0xF3 0x64 is the prefix 'REP'
				__emit 0x64;
				__emit 0xCC;	// this gets skipped over if being debugged
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bIsTrapped = FALSE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// INT 2D Debug Trap
	// =======================================================================

	inline static BOOL CheckInt2DDebugTrap()
	{
		BOOL bIsTrapped = TRUE;

		__try
		{
			_asm
			{
				int 0x2D;
				nop;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bIsTrapped = FALSE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// INT 3 Debug Trap
	// =======================================================================

	inline static BOOL CheckInt3DebugTrap()
	{
		BOOL bIsTrapped = TRUE;

		__try
		{
			_asm
			{
				int 3;	// 0xCC standard software breakpoint
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bIsTrapped = FALSE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// DebugBreak Trap
	// =======================================================================

	inline static BOOL CheckDebugBreakTrap()
	{
		BOOL bIsTrapped = TRUE;

		__try
		{
			DebugBreak();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bIsTrapped = FALSE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// Intel ICE Trap
	// =======================================================================

	inline static BOOL CheckIntelICETrap()
	{
		BOOL bIsTrapped = TRUE;

		__try
		{
			__asm __emit 0xF1;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bIsTrapped = FALSE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// Stack Segment Register Trap
	// =======================================================================

	inline static BOOL CheckStackSegmentRegisterTrap()
	{
		BOOL bIsTrapped = FALSE;

		__asm
		{
			push ss
			pop ss
			pushf
			test byte ptr[esp + 1], 1
			jz movss_not_being_debugged
		}

		bIsTrapped = TRUE;

	movss_not_being_debugged:
		// restore stack
		__asm popf;

		return bIsTrapped;
	}

	// =======================================================================
	// POPF Trap
	// =======================================================================

	inline static BOOL CheckPOPFTrap()
	{
		BOOL bIsTrapped = TRUE;

		__try
		{
			__asm
			{
				pushfd
				mov dword ptr[esp], 0x100
				popfd
				nop
			}
		}
		__except (GetExceptionCode() == EXCEPTION_SINGLE_STEP
			? EXCEPTION_EXECUTE_HANDLER
			: EXCEPTION_CONTINUE_EXECUTION)
		{
			bIsTrapped = FALSE;
		}

		return bIsTrapped;
	}

	// =======================================================================
	// Check Is Trapped
	// =======================================================================

	inline static ReportCode IsTrapped()
	{
#ifdef VMPROTECT
		VMProtectBeginMutation("Trap::IsTrapped");
#endif

		if (CheckCloseHandleExceptionTrap())
		{
#ifdef DEBUG_LOG
			Print("CheckCloseHandleExceptionTrap");
#endif
			return REPORT_CODE_DETECT_CLOSE_HANDLE_EXCEPTION_TRAP;
		}

		//	if (CheckPrefixHopTrap())
		//	{
		//#ifdef DEBUG_LOG
		//		Print("CheckPrefixHopTrap");
		//#endif
		//		return REPORT_CODE_DETECT_PREFIX_HOP_TRAP;
		//	}

		//	if (CheckInt2DDebugTrap())
		//	{
		//#ifdef DEBUG_LOG
		//		Print("CheckInt2DDebugTrap");
		//#endif
		//		return REPORT_CODE_DETECT_INT2D_DEBUG_TRAP;
		//	}

		//	if (CheckInt3DebugTrap())
		//	{
		//#ifdef DEBUG_LOG
		//		Print("CheckInt3DebugTrap");
		//#endif
		//		return REPORT_CODE_DETECT_INT3D_DEBUG_TRAP;
		//	}

		if (CheckDebugBreakTrap())
		{
#ifdef DEBUG_LOG
			Print("CheckDebugBreakTrap");
#endif
			return REPORT_CODE_DETECT_DEBUG_BREAK_TRAP;
		}

		if (CheckIntelICETrap())
		{
#ifdef DEBUG_LOG
			Print("CheckIntelICETrap");
#endif
			return REPORT_CODE_DETECT_INTEL_ICE_TRAP;
		}

		if (CheckStackSegmentRegisterTrap())
		{
#ifdef DEBUG_LOG
			Print("CheckStackSegmentRegisterTrap");
#endif
			return REPORT_CODE_DETECT_STACK_SEGMENT_REGISTER_TRAP;
		}

		if (CheckPOPFTrap())
		{
#ifdef DEBUG_LOG
			Print("CheckPOPFTrap");
#endif
			return REPORT_CODE_DETECT_POPF_TRAP;
		}

		return REPORT_CODE_NONE;

#ifdef VMPROTECT
		VMProtectEnd();
#endif
	}
}