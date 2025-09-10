#include "AddressManager.h"

#include <iostream>

#include "../Core/Config.h" // For TARGET_PROCESS_NAME
#include "../Utils/PatternScanner.h"

namespace kx {

uintptr_t AddressManager::m_agentArray = 0;
uintptr_t AddressManager::m_worldViewContextPtr = 0;
uintptr_t AddressManager::m_bgfxContextFunc = 0;
uintptr_t AddressManager::m_contextCollectionFunc = 0;
uintptr_t AddressManager::m_gameThreadUpdateFunc = 0;
void* AddressManager::s_pContextCollection = nullptr;

// A helper function to resolve RIP-relative addresses (like in LEA, MOV, and CALL instructions)
uintptr_t ResolveRelativeAddress(uintptr_t instructionAddress, size_t instructionSize) {
    if (!instructionAddress) return 0;
    // The relative offset is always in the last 4 bytes of the instruction
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(instructionAddress + (instructionSize - 4));
    // The address is calculated from the instruction *after* the current one.
    return instructionAddress + instructionSize + relativeOffset;
}

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

void AddressManager::ScanGameThreadUpdateFunc() {
    // 1. Find our unique anchor pattern, which starts at the LEA for "ViewAdvanceAlert".
    std::optional<uintptr_t> locatorOpt = kx::PatternScanner::FindPattern(
        std::string(kx::ALERT_CONTEXT_LOCATOR_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!locatorOpt) {
        std::cerr << "[AddressManager] ERROR: AlertContext locator pattern not found." << std::endl;
        m_gameThreadUpdateFunc = 0;
        return;
    }

    // 2. The CALL to the getter for the AlertContext (`FUN_012ef220`) is at a fixed offset
    //    of -0x19 bytes *before* the start of our found pattern.
    uintptr_t callToGetterAddr = *locatorOpt - 0x19;

    // 3. Resolve the relative CALL to find the getter function's address.
    uintptr_t getterFuncAddr = ResolveRelativeAddress(callToGetterAddr, 5);
    if (!getterFuncAddr) {
        m_gameThreadUpdateFunc = 0;
        return;
    }

    // 4. The getter function contains a relative MOV. Resolve it to find the static address
    //    that holds the pointer to the AlertContext instance.
    uintptr_t staticPtrAddr = ResolveRelativeAddress(getterFuncAddr, 7);
    if (!staticPtrAddr) {
        m_gameThreadUpdateFunc = 0;
        return;
    }

    // 5. Dereference the static pointer to get the actual AlertContext instance pointer.
    uintptr_t instancePtr = *reinterpret_cast<uintptr_t*>(staticPtrAddr);
    if (!instancePtr) {
        m_gameThreadUpdateFunc = 0;
        return;
    }

    // 6. The first 8 bytes of the instance is a pointer to its vtable.
    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(instancePtr);
    if (!vtable) {
        m_gameThreadUpdateFunc = 0;
        return;
    }

    // 7. Our target for the hook is the first function in the vtable (index 0).
    m_gameThreadUpdateFunc = vtable[0];

    std::cout << "[AddressManager] -> SUCCESS: GameThreadUpdate function resolved to: 0x" << std::hex << m_gameThreadUpdateFunc << std::dec << std::endl;
}

void AddressManager::Scan() {
    std::cout << "[AddressManager] Scanning for memory addresses..." << std::endl;
    ScanAgentArray();

	// currently unused
    /*ScanWorldViewContextPtr();
    ScanBgfxContextFunc();*/
    ScanContextCollectionFunc();
    ScanGameThreadUpdateFunc();
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

uintptr_t AddressManager::GetGameThreadUpdateFunc() {
    return m_gameThreadUpdateFunc;
}

void* AddressManager::GetContextCollectionPtr()
{
    return s_pContextCollection;
}
} // namespace kx