#pragma once
class HookHandler
{
public:
	static void Start();
	static void Stop();


	static void HookRecvAddress();
	static void HookSendAddress();

	static void MainProcess();
	static void RecvProcess(BYTE* byBuffer, DWORD dwLength);
	static void SendProcess(BYTE* byBuffer, DWORD dwLength);

	static DWORD GetRecvHookAddress();
	static DWORD GetSendHookAddress();

protected:
	inline static bool m_bWorking;
	inline static DWORD m_dwRecvHookAddress;
	inline static DWORD m_dwSendHookAddress;
};

