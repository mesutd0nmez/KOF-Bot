#pragma once

#define	INI_SECTION_START	'['
#define	INI_SECTION_END		']'
#define	INI_KEY_SEPARATOR	'='
#define	INI_NEWLINE			"\n"

class Ini
{
public:
	std::function<void()> onSaveEvent;

private:
	bool m_isMemory;
	std::string m_szFileName;
	// Defines key/value pairs within sections
	typedef std::map<std::string, std::string> ConfigEntryMap;

	// Defines the sections containing the key/value pairs
	typedef std::map<std::string, ConfigEntryMap> ConfigMap;

	ConfigMap m_configMap;

public:
	Ini();
	~Ini();
	Ini(const char* lpFilename);

	bool Load(const char* lpFileName = nullptr);
	bool Load(std::string szData);
	void Save(const char* lpFileName = nullptr);
	void Reset();

	std::string Dump();

	int GetInt(const char* lpAppName, const char* lpKeyName, const int nDefault);
	std::unordered_set<int> GetInt(const char* lpAppName, const char* lpKeyName, const std::unordered_set<int> nDefault);

	int SetInt(const char* lpAppName, const char* lpKeyName, const int nDefault);
	std::unordered_set<int> SetInt(const char* lpAppName, const char* lpKeyName, const std::unordered_set<int> nDefault);

	bool GetBool(const char* lpAppName, const char* lpKeyName, const bool bDefault);
	void GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, std::string& lpOutString, bool bAllowEmptyStrings = true);

	std::string GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, bool bAllowEmptyStrings = true);
	std::unordered_set<std::string> GetString(const char* lpAppName, const char* lpKeyName, const std::unordered_set<std::string> nDefault);

	const char* SetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault);
	std::unordered_set<std::string> SetString(const char* lpAppName, const char* lpKeyName, const std::unordered_set<std::string> nDefault);

	ConfigMap* GetConfigMap() { return &m_configMap; };
};

