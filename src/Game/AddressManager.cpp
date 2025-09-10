#include "AddressManager.h"

#include <iostream>

#include "../Core/Config.h" // For TARGET_PROCESS_NAME
#include "../Utils/PatternScanner.h"

namespace kx {

uintptr_t AddressManager::m_agentArray = 0;
uintptr_t AddressManager::m_worldViewContextPtr = 0;
uintptr_t AddressManager::m_bgfxContextFunc = 0;
uintptr_t AddressManager::m_contextCollectionFunc = 0;

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
        std::cout << "[AddressManager] -> SUCCESS: WorldViewContext resolved to: 0x" << std::hex << m_worldViewContextPtr << std::dec << std::endl;
    }
    else {
        std::cerr << "[AddressManager] ERROR: WvContext static address was null." << std::endl;
    }
}

void AddressManager::ScanBgfxContextFunc()
{
    std::optional<uintptr_t> getContextOpt = kx::PatternScanner::FindPattern(
        std::string(kx::BGFX_CONTEXT_FUNC_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!getContextOpt) {
        std::cerr << "[AddressManager] ERROR: BGFX Context function pattern not found." << std::endl;
        m_bgfxContextFunc = 0;
        return;
    }

    // The pattern is located inside the function. We must subtract the offset
    // to get the address of the function's first instruction.
    // Start of function: 00b41ef0
    // Start of pattern:  00b41f25
    // Offset = 0x35
    uintptr_t patternAddress = *getContextOpt;
    m_bgfxContextFunc = patternAddress - 0x35;

    std::cout << "[AddressManager] -> SUCCESS: BGFX Context function resolved to: 0x" << std::hex << m_bgfxContextFunc << std::dec << std::endl;
}

void AddressManager::ScanContextCollectionFunc()
{
    std::optional<uintptr_t> funcOpt = kx::PatternScanner::FindPattern(
        std::string(kx::CONTEXT_COLLECTION_FUNC_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!funcOpt) {
        std::cerr << "[AddressManager] ERROR: ContextCollection function pattern not found." << std::endl;
        m_contextCollectionFunc = 0;
        return;
    }

    m_contextCollectionFunc = *funcOpt;
    std::cout << "[AddressManager] -> SUCCESS: ContextCollection function resolved to: 0x" << std::hex << m_contextCollectionFunc << std::dec << std::endl;
}

void AddressManager::Scan() {
    std::cout << "[AddressManager] Scanning for memory addresses..." << std::endl;
    ScanAgentArray();

	// currently unused
    /*ScanWorldViewContextPtr();
    ScanBgfxContextFunc();
    ScanContextCollectionFunc();*/
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

uintptr_t AddressManager::GetBgfxContextFunc()
{
    return m_bgfxContextFunc;
}

uintptr_t AddressManager::GetContextCollectionFunc()
{
    return m_contextCollectionFunc;
}
} // namespace kx