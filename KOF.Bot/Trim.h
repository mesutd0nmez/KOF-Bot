#pragma once

// trim from start (in place)
inline static void ltrim(std::string& s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) 
        {
        return !std::isspace(ch);
        }));
}

// trim from end (in place)
inline static void rtrim(std::string& s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) 
        {
        return !std::isspace(ch);
        }).base(), s.end());
}

// trim from both ends (in place)
inline static void trim(std::string& s) 
{
    rtrim(s);
    ltrim(s);
}

// trim from start (copying)
inline static std::string ltrim_copy(std::string s) 
{
    ltrim(s);
    return s;
}

// trim from end (copying)
inline static std::string rtrim_copy(std::string s) 
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
inline static std::string trim_copy(std::string s) 
{
    trim(s);
    return s;
}