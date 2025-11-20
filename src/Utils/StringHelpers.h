#pragma once

#include <string>
#include <cwchar>
#include <windows.h>

namespace kx::StringHelpers {

inline std::string WCharToUTF8String(const wchar_t* wstr) {
    if (!wstr || *wstr == L'\0') {
        return {};
    }

    constexpr size_t MAX_LEN = 4096;
    const size_t length = wcsnlen(wstr, MAX_LEN);
    if (length == 0 || length >= MAX_LEN) {
        return {};
    }

    const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(length), nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        return {};
    }

    std::string result(sizeNeeded, '\0');
    const int written = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(length), result.data(), sizeNeeded, nullptr, nullptr);
    if (written <= 0) {
        return {};
    }

    if (written < sizeNeeded) {
        result.resize(written);
    }
    return result;
}

} // namespace kx::StringHelpers
