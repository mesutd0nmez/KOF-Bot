// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

std::string to_string(wchar_t const* wcstr)
{
    auto s = std::mbstate_t();
    auto const target_char_count = std::wcsrtombs(nullptr, &wcstr, 0, &s);
    if (target_char_count == static_cast<std::size_t>(-1))
    {
        throw std::logic_error("Illegal byte sequence");
    }

    // +1 because std::string adds a null terminator which isn't part of size
    auto str = std::string(target_char_count, '\0');
    std::wcsrtombs(const_cast<char*>(str.data()), &wcstr, str.size() + 1, &s);
    return str;
}

std::string to_string(std::wstring const& wstr)
{
    return to_string(wstr.c_str());
}
