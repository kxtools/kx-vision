#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <optional>

namespace kx {

class PatternScanner {
public:
    // Scans a memory region for a given byte pattern.
    // pattern: IDA-style pattern string (e.g., "48 89 5C 24 ? 57 48 83 EC 20")
    // moduleName: Name of the module to scan within the current process.
    // Returns the address of the first match, or std::nullopt if not found.
    static std::optional<uintptr_t> FindPattern(const std::string& pattern, const std::string& moduleName);

    // Scans a specific memory range for a given byte pattern.
    static std::optional<uintptr_t> FindPattern(const std::string& pattern, uintptr_t startAddress, size_t scanSize);

private:
    // Helper to convert pattern string to bytes and mask.
    static bool PatternToBytes(const std::string& pattern, std::vector<int>& bytes);
};

}
