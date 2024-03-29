#include "pch.h"
#include "Ini.h"
#include "Trim.h"

#define INI_BUFFER 512

Ini::Ini()
{
    m_configMap.clear();

    m_isMemory = true;
    m_szFileName.clear();
}

Ini::~Ini()
{
    m_configMap.clear();

    m_isMemory = true;
    m_szFileName.clear();
}

Ini::Ini(const char* lpFilename)
{
    m_szFileName = lpFilename;
    m_isMemory = false;

    Load(lpFilename);
}

bool Ini::Load(const char* lpFilename)
{
    m_szFileName = lpFilename;
    m_isMemory = false;
    const char* fn = (lpFilename == nullptr ? m_szFileName.c_str() : lpFilename);
    std::ifstream file(fn);
    if (!file)
    {
#ifdef DEBUG_LOG
        Print("%s does not exist, will use configured defaults", fn);
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
        // at the expense of not being able to use '=' in section names.
        // As this is uncommon behavior, this is a suitable trade-off.
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
        // at the expense of not being able to use '=' in section names.
        // As this is uncommon behavior, this is a suitable trade-off.
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

void Ini::Save(const char* lpFilename)
{
    if (!m_isMemory)
    {
        const char* fn = (lpFilename == nullptr ? m_szFileName.c_str() : lpFilename);
        FILE* fp = fopen(fn, "w");

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
    else
    {
        if (onSaveEvent)
            onSaveEvent();
    }
}

void Ini::Reset()
{
    m_configMap.clear();
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
    else
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        sectionItr = m_configMap.find(lpAppName);

        char tmpDefault[INI_BUFFER] = "";
        _snprintf(tmpDefault, INI_BUFFER, "%d", nDefault);
        sectionItr->second[lpKeyName] = tmpDefault;
    }

    return nDefault;
}

std::unordered_set<int> Ini::GetInt(const char* lpAppName, const char* lpKeyName, const std::unordered_set<int> nDefault)
{
    ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

    if (sectionItr != m_configMap.end())
    {
        ConfigEntryMap::iterator keyItr = sectionItr->second.find(lpKeyName);

        if (keyItr != sectionItr->second.end())
        {
            std::unordered_set<int> v;
            std::stringstream ss(keyItr->second.c_str());

            for (int i; ss >> i;)
            {
                v.insert(i);

                if (ss.peek() == ',')
                    ss.ignore();
            }

            return v;
        }
    }
    else
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        sectionItr = m_configMap.find(lpAppName);

        std::string ret;

        for (const int& s : nDefault)
        {
            if (!ret.empty())
                ret += ",";

            ret += std::to_string(s);
        }

        sectionItr->second[lpKeyName] = ret.c_str();
    }

    return nDefault;
}

int Ini::SetInt(const char* lpAppName, const char* lpKeyName, const int nDefault)
{
    char tmpDefault[INI_BUFFER];
    _snprintf(tmpDefault, INI_BUFFER, "%d", nDefault);
    SetString(lpAppName, lpKeyName, tmpDefault);
    return nDefault;
}

std::unordered_set<int> Ini::SetInt(const char* lpAppName, const char* lpKeyName, const std::unordered_set<int> nDefault)
{
    std::string ret;

    for (const int& s : nDefault)
    {
        if (!ret.empty())
            ret += ",";

        ret += std::to_string(s);
    }

    ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

    if (sectionItr == m_configMap.end()) 
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        sectionItr = m_configMap.find(lpAppName);
    }

    ConfigEntryMap& entryMap = sectionItr->second;
    auto keyItr = entryMap.find(lpKeyName);

    std::string strDefault = ret.c_str();
    if (keyItr != entryMap.end() 
        && keyItr->second == strDefault) 
    {
        return nDefault;
    }

    entryMap[lpKeyName] = strDefault;

    Save();

    return nDefault;
}

bool Ini::GetBool(const char* lpAppName, const char* lpKeyName, const bool bDefault)
{
    return GetInt(lpAppName, lpKeyName, bDefault) == 1;
}

void Ini::GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, std::string& lpOutString, bool bAllowEmptyStrings /*= true*/)
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
    else
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        sectionItr = m_configMap.find(lpAppName);
        sectionItr->second[lpKeyName] = lpDefault;
    }

    lpOutString = lpDefault;
}

std::string Ini::GetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault, bool bAllowEmptyStrings /*= true*/)
{
    std::string lpOutString;
    GetString(lpAppName, lpKeyName, lpDefault, lpOutString, bAllowEmptyStrings);
    return lpOutString;
}

std::unordered_set<std::string> Ini::GetString(const char* lpAppName, const char* lpKeyName, const std::unordered_set<std::string> nDefault)
{
    ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

    if (sectionItr != m_configMap.end())
    {
        ConfigEntryMap::iterator keyItr = sectionItr->second.find(lpKeyName);

        if (keyItr != sectionItr->second.end())
        {
            std::unordered_set<std::string> v;
            std::stringstream ss(keyItr->second);

            for (std::string s; std::getline(ss, s, ',');)
            {
                v.insert(s.c_str());
            }

            return v;
        }
    }
    else
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        sectionItr = m_configMap.find(lpAppName);

        std::string ret;

        for (const std::string& s : nDefault)
        {
            if (!ret.empty())
                ret += ",";

            ret += s;
        }

        sectionItr->second[lpKeyName] = ret.c_str();
    }

    return nDefault;
}


const char* Ini::SetString(const char* lpAppName, const char* lpKeyName, const char* lpDefault)
{
    ConfigMap::iterator itr = m_configMap.find(lpAppName);

    if (itr == m_configMap.end()) 
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        itr = m_configMap.find(lpAppName);
    }

    ConfigEntryMap& entryMap = itr->second;
    auto keyItr = entryMap.find(lpKeyName);

    if (keyItr != entryMap.end() 
        && keyItr->second == lpDefault) 
    {
        return lpDefault;
    }

    entryMap[lpKeyName] = lpDefault;
    Save();

    return lpDefault;
}

std::unordered_set<std::string> Ini::SetString(const char* lpAppName, const char* lpKeyName, const std::unordered_set<std::string> nDefault)
{
    std::string ret;

    for (const std::string& s : nDefault)
    {
        if (!ret.empty())
            ret += ",";

        ret += s;
    }

    ConfigMap::iterator sectionItr = m_configMap.find(lpAppName);

    if (sectionItr == m_configMap.end())
    {
        m_configMap.insert(std::make_pair(lpAppName, ConfigEntryMap()));
        sectionItr = m_configMap.find(lpAppName);
    }

    ConfigEntryMap& entryMap = sectionItr->second;
    auto keyItr = entryMap.find(lpKeyName);

    std::string strDefault = ret.c_str();
    if (keyItr != entryMap.end()
        && keyItr->second == strDefault)
    {
        return nDefault;
    }

    entryMap[lpKeyName] = strDefault;

    Save();

    return nDefault;
}
