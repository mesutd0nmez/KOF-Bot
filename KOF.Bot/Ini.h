#pragma once
#include <map>
#include <string>
#include <vector>

#define	INI_SECTION_START	'['
#define	INI_SECTION_END		']'
#define	INI_KEY_SEPARATOR	'='
#define	INI_NEWLINE			"\n"

class Ini
{
public:
	std::function<void()> onSaveEvent;

private:
	std::mutex m_mutexMap;

private:
	bool m_isMemory;
	std::string m_szFileName;
	// Defines key/value pairs within sections
	typedef std::map<std::string, std::string> ConfigEntryMap;

	// Defines the sections containing the key/value pairs
	typedef std::map<std::string, ConfigEntryMap> ConfigMap;

	ConfigMap m_configMap;

	std::recursive_mutex m_mutex;

public:
	Ini();
	Ini(const char* lpFilename);

	bool Load(const char* lpFileName = nullptr);
	bool Load(std::string szData);
	void Save(const char* lpFileName = nullptr);

	std::string Dump();

	int GetInt(const char* lpAppName, const char* lpKeyName, const int nDefault);
	std::vector<int> GetInt(const char* lpAppName, const char* lpKeyName, const std::vector<int> nDefault);

	int SetInt(const char* lpAppName, const char* lpKeyName, const int nDefault);
	std::vector<int> SetInt(const char* lpAppName, const char* lpKeyName, const std::vector<int> nDefault);

	bool GetBool(const char* lpAppName, const char* lpKeyName, const bool bDefault);
	void GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, std::string& lpOutString, bool bAllowEmptyStrings = true);
	std::string GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, bool bAllowEmptyStrings = true);

	const char* SetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault);

	ConfigMap* GetConfigMap() { return &m_configMap; };
};

