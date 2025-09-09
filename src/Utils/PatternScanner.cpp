#include "PatternScanner.h"

#include <iostream> // For error logging (temporary, consider a proper logger)
#include <optional>
#include <psapi.h> // For GetModuleInformation
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib, "psapi.lib") // Link against psapi.lib for GetModuleInformation

namespace kx {

// Helper to convert pattern string to bytes and mask.
// Returns true on success, false on parsing error.
bool PatternScanner::PatternToBytes(const std::string& pattern, std::vector<int>& bytes) {
    bytes.clear();
    std::stringstream ss(pattern);
    std::string byteStr;

    while (ss >> byteStr) {
        if (byteStr == "?" || byteStr == "??") {
            bytes.push_back(-1); // Use -1 to represent a wildcard byte
        } else {
            try {
                int byteVal = std::stoi(byteStr, nullptr, 16);
                if (byteVal >= 0 && byteVal <= 255) {
                    bytes.push_back(byteVal);
                } else {
                    // Invalid byte value
                    std::cerr << "[PatternScanner] Error: Invalid byte value '" << byteStr << "' in pattern." << std::endl;
                    return false;
                }
            } catch (const std::invalid_argument&) {
                // Invalid hex string format
                 std::cerr << "[PatternScanner] Error: Invalid hex string '" << byteStr << "' in pattern." << std::endl;
                return false;
            } catch (const std::out_of_range&) {
                // Hex string out of range for int
                 std::cerr << "[PatternScanner] Error: Hex string '" << byteStr << "' out of range in pattern." << std::endl;
                return false;
            }
        }
    }
    return !bytes.empty(); // Ensure the pattern wasn't empty or just whitespace
}


std::optional<uintptr_t> PatternScanner::FindPattern(const std::string& pattern, const std::string& moduleName) {
    std::vector<int> patternBytes;
    if (!PatternToBytes(pattern, patternBytes)) {
        std::cerr << "[PatternScanner] Failed to parse pattern string." << std::endl;
        return std::nullopt;
    }

    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    if (hModule == NULL) {
        std::cerr << "[PatternScanner] Error: Could not get handle for module '" << moduleName << "'. Error code: " << GetLastError() << std::endl;
        return std::nullopt;
    }

    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        std::cerr << "[PatternScanner] Error: Could not get module information for '" << moduleName << "'. Error code: " << GetLastError() << std::endl;
        return std::nullopt;
    }

    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    uintptr_t scanSize = moduleInfo.SizeOfImage;
    
    return FindPattern(pattern, baseAddress, scanSize);
}

std::optional<uintptr_t> PatternScanner::FindPattern(const std::string& pattern, uintptr_t startAddress, size_t scanSize) {
    std::vector<int> patternBytes;
    if (!PatternToBytes(pattern, patternBytes)) {
        std::cerr << "[PatternScanner] Failed to parse pattern string." << std::endl;
        return std::nullopt;
    }

    size_t patternSize = patternBytes.size();

    if (scanSize < patternSize) {
         std::cerr << "[PatternScanner] Error: Module size is smaller than pattern size." << std::endl;
        return std::nullopt; // Cannot possibly find the pattern
    }

    for (uintptr_t i = 0; i <= scanSize - patternSize; ++i) {
        bool found = true;
        for (size_t j = 0; j < patternSize; ++j) {
            // Check if the byte is a wildcard (-1) or if the memory matches the pattern byte
            if (patternBytes[j] != -1 && patternBytes[j] != *reinterpret_cast<unsigned char*>(startAddress + i + j)) {
                found = false;
                break;
            }
        }

        if (found) {
            return startAddress + i;
        }
    }

    // Pattern not found
    std::cerr << "[PatternScanner] Pattern not found in specified memory range." << std::endl;
    return std::nullopt;
}


}
