#include "AddressManager.h"

#include <iostream>

#include "../Core/Config.h" // For TARGET_PROCESS_NAME
#include "../Utils/PatternScanner.h"
#include "ReClassStructs.h" // For ContextCollection and ChCliContext

namespace kx {

// Define the single static instance of the GamePointers struct.
GamePointers AddressManager::s_pointers;

// A helper function to resolve RIP-relative addresses (like in LEA, MOV, and CALL instructions)
uintptr_t ResolveRelativeAddress(uintptr_t instructionAddress, size_t instructionSize) {
    if (!instructionAddress || instructionSize < 4) return 0;
    // The relative offset is read from the last 4 bytes of the instruction.
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(instructionAddress + (instructionSize - 4));
    // The address is calculated from the instruction *after* the current one.
    return instructionAddress + instructionSize + relativeOffset;
}

void AddressManager::SetContextCollectionPtr(void* ptr)
{
    s_pointers.pContextCollection = ptr;
}

void AddressManager::ScanAgentArray() {
    std::optional<uintptr_t> avContextFuncOpt = kx::PatternScanner::FindPattern(
        std::string(kx::AGENT_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!avContextFuncOpt) {
        std::cerr << "[AddressManager] ERROR: AgentViewContext pattern not found." << std::endl;
        s_pointers.agentArray = 0;
        return;
    }

    uintptr_t avContextFuncAddr = *avContextFuncOpt;
    std::cout << "[AddressManager] Found AgentViewContext at: 0x" << std::hex << avContextFuncAddr << std::dec << std::endl;

    std::optional<uintptr_t> leaInstructionOpt = kx::PatternScanner::FindPattern(std::string(kx::AGENT_ARRAY_LEA_PATTERN), avContextFuncAddr, 0x300);

    if (!leaInstructionOpt) {
        std::cerr << "[AddressManager] ERROR: Could not find AgentArray LEA instruction inside AvContext." << std::endl;
        s_pointers.agentArray = 0;
        return;
    }

    uintptr_t leaInstructionAddress = *leaInstructionOpt;
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(leaInstructionAddress + 3);
    uintptr_t addressOfNextInstruction = leaInstructionAddress + 7;
    uintptr_t agentStructBase = addressOfNextInstruction + relativeOffset;
    s_pointers.agentArray = agentStructBase + 0x8;

    std::cout << "[AddressManager] -> SUCCESS: AgentArray resolved to: 0x" << std::hex << s_pointers.agentArray << std::dec << std::endl;
}

void AddressManager::ScanWorldViewContextPtr() {
    std::optional<uintptr_t> landmarkOpt = kx::PatternScanner::FindPattern(
        std::string(kx::WORLD_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!landmarkOpt) {
        std::cerr << "[AddressManager] ERROR: WorldViewContext pattern not found." << std::endl;
        s_pointers.worldViewContextPtr = 0;
        return;
    }

    uintptr_t landmarkAddress = *landmarkOpt;
    uintptr_t movInstructionAddr = landmarkAddress - 7;

    int32_t relativeOffset = *reinterpret_cast<int32_t*>(movInstructionAddr + 3);
    uintptr_t addressOfNextInstruction = movInstructionAddr + 7;
    uintptr_t staticPointerAddress = addressOfNextInstruction + relativeOffset;

    s_pointers.worldViewContextPtr = *reinterpret_cast<uintptr_t*>(staticPointerAddress);

    if (s_pointers.worldViewContextPtr) {
        std::cout << "[AddressManager] -> SUCCESS: WorldViewContext resolved to: 0x" << std::hex << s_pointers.worldViewContextPtr << std::dec << std::endl;
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
        s_pointers.bgfxContextFunc = 0;
        return;
    }

    // The pattern is located inside the function. We must subtract the offset
    // to get the address of the function's first instruction.
    // Start of function: 00b41ef0
    // Start of pattern:  00b41f25
    // Offset = 0x35
    uintptr_t patternAddress = *getContextOpt;
    s_pointers.bgfxContextFunc = patternAddress - 0x35;

    std::cout << "[AddressManager] -> SUCCESS: BGFX Context function resolved to: 0x" << std::hex << s_pointers.bgfxContextFunc << std::dec << std::endl;
}

void AddressManager::ScanContextCollectionFunc()
{
    std::optional<uintptr_t> funcOpt = kx::PatternScanner::FindPattern(
        std::string(kx::CONTEXT_COLLECTION_FUNC_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!funcOpt) {
        std::cerr << "[AddressManager] ERROR: ContextCollection function pattern not found." << std::endl;
        s_pointers.contextCollectionFunc = 0;
        return;
    }

    s_pointers.contextCollectionFunc = *funcOpt;
    std::cout << "[AddressManager] -> SUCCESS: ContextCollection function resolved to: 0x" << std::hex << s_pointers.contextCollectionFunc << std::dec << std::endl;
}

void AddressManager::ScanGameThreadUpdateFunc() {
    std::optional<uintptr_t> locatorOpt = kx::PatternScanner::FindPattern(
        std::string(kx::ALERT_CONTEXT_LOCATOR_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!locatorOpt) {
        std::cerr << "[AddressManager] ERROR: AlertContext locator pattern not found." << std::endl;
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    uintptr_t callToGetterAddr = *locatorOpt - 0x19;

    uintptr_t getterFuncAddr = ResolveRelativeAddress(callToGetterAddr, 5);
    if (!getterFuncAddr) {
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    uintptr_t staticPtrAddr = ResolveRelativeAddress(getterFuncAddr, 7);
    if (!staticPtrAddr) {
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    uintptr_t instancePtr = *reinterpret_cast<uintptr_t*>(staticPtrAddr);
    if (!instancePtr) {
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(instancePtr);
    if (!vtable) {
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    s_pointers.gameThreadUpdateFunc = vtable[0];

    std::cout << "[AddressManager] -> SUCCESS: GameThreadUpdate function resolved to: 0x" << std::hex << s_pointers.gameThreadUpdateFunc << std::dec << std::endl;
}

void AddressManager::Scan() {
    std::cout << "[AddressManager] Scanning for memory addresses..." << std::endl;
    ScanContextCollectionFunc();
    ScanGameThreadUpdateFunc();

    // currently unused
    //ScanAgentArray();
    //ScanWorldViewContextPtr();
    //ScanBgfxContextFunc();
}

void AddressManager::Initialize() {
    Scan();
}

void* AddressManager::GetLocalPlayer() {
    if (!s_pointers.pContextCollection) return nullptr;
    
    return GetLocalPlayerImpl(s_pointers.pContextCollection);
}

// Implementation function to avoid object unwinding issues
void* AddressManager::GetLocalPlayerImpl(void* pContextCollection) {
    __try {
        ReClass::ContextCollection contextCollection(pContextCollection);
        ReClass::ChCliContext chContext = contextCollection.GetChCliContext();
        if (!chContext.data()) return nullptr;
        
        return chContext.GetLocalPlayer();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

} // namespace kx
