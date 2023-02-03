#pragma once

class LoginHandler
{
public:
	static void Start();
	static void Stop();

	static void MainProcess();
	static void IntroSkipProcess();
	static void LoginProcess();
	static void ServerSelectProcess();
	static void CharacterSelectProcess();

protected:
	inline static bool m_bWorking;
};