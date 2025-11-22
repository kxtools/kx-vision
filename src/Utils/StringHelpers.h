#pragma once

#include <windows.h>
#include <span>

namespace kx::StringHelpers {

/**
 * @brief Converts WChar to UTF8 directly into a pre-allocated buffer.
 * Zero allocations. Safe truncation if buffer is too small.
 * @param wstr Source wide string.
 * @param buffer Destination buffer (fixed array or span).
 * @return Number of bytes written (excluding null terminator).
 */
inline size_t WriteWCharToUTF8(const wchar_t* wstr, std::span<char> buffer) {
    if (buffer.empty()) {
        return 0;
    }
    
    if (!wstr) {
        buffer[0] = '\0';
        return 0;
    }

    int result = WideCharToMultiByte(
        CP_UTF8, 
        0, 
        wstr, 
        -1,
        buffer.data(), 
        static_cast<int>(buffer.size()), 
        nullptr, 
        nullptr
    );

    if (result > 0) {
        return static_cast<size_t>(result) - 1;
    }
    else {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer.back() = '\0';
            return buffer.size() - 1;
        }
        
        buffer[0] = '\0';
        return 0;
    }
}

} // namespace kx::StringHelpers
