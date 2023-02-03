#pragma once
#include "pch.h"
#include "Ini.h"
#include "HookHandler.h"
#include "LoginHandler.h"
#include "AttackHandler.h"
#include "ProtectionHandler.h"
#include "ConfigHandler.h"
#include "TableHandler.h"
#include "Client.h"

class Bot
{
public:
	Bot();
	virtual ~Bot();

	static void Start();
	static void Stop();

private:
	static void InitializeStaticData();
	static void InitializeAddress();

protected:
	inline static Client m_Client;

	inline static AttackHandler m_AttackHandler;
	inline static ConfigHandler m_ConfigHandler;
	inline static HookHandler m_HookHandler;
	inline static LoginHandler m_LoginHandler;
	inline static ProtectionHandler m_ProtectionHandler;
	inline static TableHandler m_TableHandler;

	inline static Ini* m_AddressConfig;
};

