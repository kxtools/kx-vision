#include "AddressManager.h"

#include <iostream>

#include "../Core/Config.h" // For TARGET_PROCESS_NAME
#include "../Utils/PatternScanner.h"

namespace kx {

uintptr_t AddressManager::m_agentArray = 0;
uintptr_t AddressManager::m_worldViewContextPtr = 0;

void AddressManager::ScanAgentArray() {
    std::optional<uintptr_t> avContextFuncOpt = kx::PatternScanner::FindPattern(
        std::string(kx::AGENT_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!avContextFuncOpt) {
        std::cerr << "[AddressManager] ERROR: AgentViewContext pattern not found." << std::endl;
        m_agentArray = 0;
        return;
    }

    uintptr_t avContextFuncAddr = *avContextFuncOpt;
    std::cout << "[AddressManager] Found AgentViewContext at: 0x" << std::hex << avContextFuncAddr << std::dec << std::endl;

    std::optional<uintptr_t> leaInstructionOpt = kx::PatternScanner::FindPattern(std::string(kx::AGENT_ARRAY_LEA_PATTERN), avContextFuncAddr, 0x300);

    if (!leaInstructionOpt) {
        std::cerr << "[AddressManager] ERROR: Could not find AgentArray LEA instruction inside AvContext." << std::endl;
        m_agentArray = 0;
        return;
    }

    uintptr_t leaInstructionAddress = *leaInstructionOpt;
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(leaInstructionAddress + 3);
    uintptr_t addressOfNextInstruction = leaInstructionAddress + 7;
    uintptr_t agentStructBase = addressOfNextInstruction + relativeOffset;
    m_agentArray = agentStructBase + 0x8;

    std::cout << "[AddressManager] -> SUCCESS: AgentArray resolved to: 0x" << std::hex << m_agentArray << std::dec << std::endl;
}

void AddressManager::ScanWorldViewContextPtr() {
    std::optional<uintptr_t> landmarkOpt = kx::PatternScanner::FindPattern(
        std::string(kx::WORLD_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!landmarkOpt) {
        std::cerr << "[AddressManager] ERROR: WorldViewContext pattern not found." << std::endl;
        m_worldViewContextPtr = 0;
        return;
    }

    uintptr_t landmarkAddress = *landmarkOpt;
    uintptr_t movInstructionAddr = landmarkAddress - 7;

    int32_t relativeOffset = *reinterpret_cast<int32_t*>(movInstructionAddr + 3);
    uintptr_t addressOfNextInstruction = movInstructionAddr + 7;
    uintptr_t staticPointerAddress = addressOfNextInstruction + relativeOffset;

    m_worldViewContextPtr = *reinterpret_cast<uintptr_t*>(staticPointerAddress);

    if (m_worldViewContextPtr) {
        std::cout << "[AddressManager] SUCCESS: WorldViewContext resolved to: 0x" << std::hex << m_worldViewContextPtr << std::dec << std::endl;
    }
    else {
        std::cerr << "[AddressManager] ERROR: WvContext static address was null." << std::endl;
    }
}

void AddressManager::Scan() {
    std::cout << "[AddressManager] Scanning for memory addresses..." << std::endl;
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