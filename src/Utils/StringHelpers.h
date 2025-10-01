#pragma once

#include <string>
#include <windows.h> // For WideCharToMultiByte

namespace kx {
namespace StringHelpers {

// Helper to convert wide-character string to UTF-8 string with enhanced safety
inline std::string WCharToUTF8String(const wchar_t* wstr) {
    if (!wstr) return "";
    
    // Check for empty string
    if (wstr[0] == L'\0') return "";
    
    // Add length limit to prevent malformed strings from causing issues
    const size_t MAX_STRING_LENGTH = 8192; // Reasonable limit
    size_t wstr_len = wcsnlen(wstr, MAX_STRING_LENGTH);
    if (wstr_len == 0) return "";
    if (wstr_len >= MAX_STRING_LENGTH) {
        // String too long, might be corrupted
        return "[STRING_TOO_LONG]";
    }

    const int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(wstr_len), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return "";

    std::string strTo(size_needed, 0);
    
    // Safe access to string data
    char* data_ptr = strTo.empty() ? nullptr : &strTo[0];
    if (!data_ptr) return "";
    
    const int result = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(wstr_len), data_ptr, size_needed, NULL, NULL);
    
    if (result <= 0) {
        return "[CONVERSION_ERROR]";
    }
    
    // Resize to actual converted length (no null terminator to remove since we didn't include it)
    strTo.resize(result);
    return strTo;
}

} // namespace StringHelpers
} // namespace kx
