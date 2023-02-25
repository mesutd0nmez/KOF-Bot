#include "pch.h"
#include "Ini.h"
#include "Trim.h"

#define INI_BUFFER 512

Ini::Ini()
{
	m_isMemory = true;
}

Ini::Ini(const char *lpFilename)
{
	m_szFileName = lpFilename;
	m_isMemory = false;

	Load(lpFilename);
}

bool Ini::Load(const char * lpFilename)
{
	m_szFileName = lpFilename;
	m_isMemory = false;
	const char * fn = (lpFilename == nullptr ? m_szFileName.c_str() : lpFilename);
	std::ifstream file(fn);
	if (!file)
	{
#ifdef DEBUG
		printf("Warning: %s does not exist, will use configured defaults.\n", fn);
#endif
		return false;
	}

	std::string currentSection;

	// If an invalid section is hit
	// Ensure that we don't place key/value pairs
	// from the invalid section into the previously loaded section.
	bool bSkipNextSection = false; 
	while (!file.eof())
	{
		std::string line;
		getline(file, line);

		rtrim(line);
		if (line.empty())
			continue;

		// Check for value strings first
		// It's faster than checking for a section
		// at the expense of of not being able to use '=' in section names.
		// As this is uncommon behaviour, this is a suitable trade-off.
		size_t keySeparatorPos = line.find(INI_KEY_SEPARATOR);
		if (keySeparatorPos != std::string::npos)
		{
			if (bSkipNextSection)
				continue;

			std::string key = line.substr(0, keySeparatorPos),
				value = line.substr(keySeparatorPos + 1);

			// Clean up key/value to allow for 'key = value'
			rtrim(key);   /* remove trailing whitespace from keys */
			ltrim(value); /* remove preleading whitespace from values */

			ConfigMap::iterator itr = m_configMap.find(currentSection);
			if (itr == m_configMap.end())
			{
				m_configMap.insert(std::make_pair(currentSection, ConfigEntryMap()));
				itr = m_configMap.find(currentSection);
			}

			itr->second[key] = value;
			continue;
		}

		// Not a value, so assume it's a section
		size_t sectionStart = line.find_first_of(INI_SECTION_START),
			sectionEnd = line.find_last_of(INI_SECTION_END);

		if (sectionStart == std::string::npos
			|| sectionEnd == std::string::npos
			|| sectionStart > sectionEnd)
		{
			/* invalid section */
			bSkipNextSection = true;
			continue;
		}

		currentSection = line.substr(sectionStart + 1, sectionEnd - 1);
		bSkipNextSection = false;
	}

	file.close();
	return true;
}

bool Ini::Load(std::string szData)
{
	m_isMemory = true;

	std::istringstream iss(szData);
	std::istream& file = iss;

	std::string currentSection;

	// If an invalid section is hit
	// Ensure that we don't place key/value pairs
	// from the invalid section into the previously loaded section.
	bool bSkipNextSection = false;
	while (!file.eof())
	{
		std::string line;
		getline(file, line);

		rtrim(line);
		if (line.empty())
			continue;

		// Check for value strings first
		// It's faster than checking for a section
		// at the expense of of not being able to use '=' in section names.
		// As this is uncommon behaviour, this is a suitable trade-off.
		size_t keySeparatorPos = line.find(INI_KEY_SEPARATOR);
		if (keySeparatorPos != std::string::npos)
		{
			if (bSkipNextSection)
				continue;

			std::string key = line.substr(0, keySeparatorPos),
				value = line.substr(keySeparatorPos + 1);

			// Clean up key/value to allow for 'key = value'
			rtrim(key);   /* remove trailing whitespace from keys */
			ltrim(value); /* remove preleading whitespace from values */

			ConfigMap::iterator itr = m_configMap.find(currentSection);
			if (itr == m_configMap.end())
			{
				m_configMap.insert(std::make_pair(currentSection, ConfigEntryMap()));
				itr = m_configMap.find(currentSection);
			}

			itr->second[key] = value;
			continue;
		}

		// Not a value, so assume it's a section
		size_t sectionStart = line.find_first_of(INI_SECTION_START),
			sectionEnd = line.find_last_of(INI_SECTION_END);

		if (sectionStart == std::string::npos
			|| sectionEnd == std::string::npos
			|| sectionStart > sectionEnd)
		{
			/* invalid section */
			bSkipNextSection = true;
			continue;
		}

		currentSection = line.substr(sectionStart + 1, sectionEnd - 1);
		bSkipNextSection = false;
	}

	return true;
}

void Ini::Save(const char * lpFilename)
{
	if (m_isMemory) return;
	const char * fn = (lpFilename == nullptr ? m_szFileName.c_str() : lpFilename);
	FILE * fp = fopen(fn, "w");
	for (auto sectionItr = m_configMap.begin(); sectionItr != m_configMap.end(); sectionItr++)
	{
		// Start the section
		fprintf(fp, "[%s]" INI_NEWLINE, sectionItr->first.c_str());

		// Now list out all the key/value pairs
		for (auto keyItr = sectionItr->second.begin(); keyItr != sectionItr->second.end(); keyItr++)
			fprintf(fp, "%s=%s" INI_NEWLINE, keyItr->first.c_str(), keyItr->second.c_str());

		// Use a trailing newline to finish the section, to make it easier to read
		fprintf(fp, INI_NEWLINE);
	}
	fclose(fp);
}

std::string Ini::Dump()
{
	std::stringstream f;
	for (auto sectionItr = m_configMap.begin(); sectionItr != m_configMap.end(); sectionItr++)
	{
		f << "[" << sectionItr->first.c_str() << "]" << INI_NEWLINE;

		for (auto keyItr = sectionItr->second.begin(); keyItr != sectionItr->second.end(); keyItr++)
			f << keyItr->first.c_str() << "=" << keyItr->second.c_str() << INI_NEWLINE;

		f << INI_NEWLINE;
	}

	return f.str().c_str();
}

int Ini::GetInt(const char* lpAppName, const char* lpKeyName, const int nDefault)
{
	ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

	if (sectionItr != m_configMap.end())
	{
		ConfigEntryMap::iterator keyItr = sectionItr->second.find(lpKeyName);

		if (keyItr != sectionItr->second.end())
			return atoi(keyItr->second.c_str());
	}

	SetInt(lpAppName, lpKeyName, nDefault);
	return nDefault;
}

std::vector<int> Ini::GetInt(const char* lpAppName, const char* lpKeyName, const std::vector<int> nDefault)
{
	ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

	if (sectionItr != m_configMap.end())
	{
		ConfigEntryMap::iterator keyItr = sectionItr->second.find(lpKeyName);

		if (keyItr != sectionItr->second.end())
		{
			std::vector<int> v;
			std::stringstream ss(keyItr->second.c_str());

			for (int i; ss >> i;) 
			{
				v.push_back(i);

				if (ss.peek() == ',')
					ss.ignore();
			}

			return v;
		}
	}

	SetInt(lpAppName, lpKeyName, nDefault);

	return nDefault;
}

int Ini::SetInt(const char* lpAppName, const char* lpKeyName, const int nDefault)
{
	char tmpDefault[INI_BUFFER];
	_snprintf(tmpDefault, INI_BUFFER, "%d", nDefault);
	SetString(lpAppName, lpKeyName, tmpDefault);
	return nDefault;
}

std::vector<int> Ini::SetInt(const char* lpAppName, const char* lpKeyName, const std::vector<int> nDefault)
{
	std::string ret;

	for (const int& s : nDefault) 
	{
		if (!ret.empty())
			ret += ",";

		ret += std::to_string(s);
	}

	SetString(lpAppName, lpKeyName, ret.c_str());

	return nDefault;
}

bool Ini::GetBool(const char* lpAppName, const char* lpKeyName, const bool bDefault)
{
	return GetInt(lpAppName, lpKeyName, bDefault) == 1;
}

void Ini::GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, std::string & lpOutString, bool bAllowEmptyStrings /*= true*/)
{
	ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

	if (sectionItr != m_configMap.end())
	{
		ConfigEntryMap::iterator keyItr = sectionItr->second.find(lpKeyName);

		if (keyItr != sectionItr->second.end())
		{
			lpOutString = keyItr->second;
			return;
		}
	}

	SetString(lpAppName, lpKeyName, lpDefault);
	lpOutString = lpDefault;
}

std::string Ini::GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, bool bAllowEmptyStrings /*= true*/)
{
	std::string lpOutString;
	GetString(lpAppName, lpKeyName, lpDefault, lpOutString, bAllowEmptyStrings);
	return lpOutString;
}

const char* Ini::SetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault)
{
	ConfigMap::iterator itr = m_configMap.find(lpAppName);
	m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
	itr = m_configMap.find(lpAppName);
	itr->second[lpKeyName] = lpDefault;
	Save();
	return lpDefault;
}
