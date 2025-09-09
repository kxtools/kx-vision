#include "AddressManager.h"
#include "PatternScanner.h"
#include "Config.h" // For TARGET_PROCESS_NAME
#include <iostream>

namespace kx {

uintptr_t AddressManager::m_agentArray = 0;
uintptr_t AddressManager::m_worldViewContextPtr = 0;

void AddressManager::ScanAgentArray() {
    std::cout << "Scanning for AgentViewContext function..." << std::endl;
    
    std::optional<uintptr_t> avContextFuncOpt = kx::PatternScanner::FindPattern(
        std::string(kx::AGENT_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!avContextFuncOpt) {
        std::cerr << "[AddressManager] AgentViewContext function pattern not found." << std::endl;
        m_agentArray = 0;
        return;
    }

    uintptr_t avContextFuncAddr = *avContextFuncOpt;
    std::cout << "[AddressManager] AgentViewContext function found at: 0x" << std::hex << avContextFuncAddr << std::dec << std::endl;

    std::optional<uintptr_t> leaInstructionOpt = kx::PatternScanner::FindPattern(std::string(kx::AGENT_ARRAY_LEA_PATTERN), avContextFuncAddr, 0x300);

    if (!leaInstructionOpt) {
        std::cerr << "[AddressManager] CRITICAL: Found AvContext, but could not find the AgentArray LEA instruction inside it." << std::endl;
        m_agentArray = 0;
        return;
    }

    uintptr_t leaInstructionAddress = *leaInstructionOpt;
    std::cout << "[AddressManager] Found AgentArray LEA instruction at: 0x" << std::hex << leaInstructionAddress << std::dec << std::endl;

    int32_t relativeOffset = *reinterpret_cast<int32_t*>(leaInstructionAddress + 3);
    uintptr_t addressOfNextInstruction = leaInstructionAddress + 7;
    uintptr_t agentStructBase = addressOfNextInstruction + relativeOffset;
    m_agentArray = agentStructBase + 0x8;

    std::cout << "[AddressManager] SUCCESS: AgentArray pointer address resolved to: 0x" << std::hex << m_agentArray << std::dec << std::endl;
}

void AddressManager::ScanWorldViewContextPtr() {
    std::cout << "Scanning for WorldViewContext accessor function..." << std::endl;
    std::optional<uintptr_t> landmarkOpt = kx::PatternScanner::FindPattern(
        std::string(kx::WORLD_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!landmarkOpt) {
        std::cerr << "[AddressManager] WorldViewContext landmark pattern not found." << std::endl;
        m_worldViewContextPtr = 0;
        return;
    }

    uintptr_t landmarkAddress = *landmarkOpt;
    uintptr_t movInstructionAddr = landmarkAddress - 7;

    // Decode the RIP-relative instruction to find the address of the static pointer.
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(movInstructionAddr + 3);
    uintptr_t addressOfNextInstruction = movInstructionAddr + 7;
    uintptr_t staticPointerAddress = addressOfNextInstruction + relativeOffset;

    // This is the direct, correct way to read the pointer.
    // We are reading a pointer (uintptr_t) from the staticPointerAddress.
    m_worldViewContextPtr = *reinterpret_cast<uintptr_t*>(staticPointerAddress);

    if (m_worldViewContextPtr) {
        std::cout << "[AddressManager] SUCCESS: WorldViewContext pointer resolved to: 0x" << std::hex << m_worldViewContextPtr << std::dec << std::endl;
    }
    else {
        std::cerr << "[AddressManager] ERROR: Found WvContext static address, but it contained a null pointer." << std::endl;
    }
}

void AddressManager::Scan() {
    ScanAgentArray();
    ScanWorldViewContextPtr();
}

void AddressManager::Initialize() {
    Scan();
}

void AddressManager::Refresh() {
    Scan();
}

uintptr_t AddressManager::GetAgentArray() {
    return m_agentArray;
}

uintptr_t AddressManager::GetWorldViewContextPtr() {
    return m_worldViewContextPtr;
}

} // namespace kx
