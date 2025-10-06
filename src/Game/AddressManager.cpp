#include "AddressManager.h"

#include <windows.h>
#include <psapi.h>

#include "../Core/Config.h" // For TARGET_PROCESS_NAME
#include "../Utils/DebugLogger.h"
#include "../Utils/PatternScanner.h"
#include "ReClassStructs.h" // For ContextCollection and ChCliContext

namespace kx {

// Define the single static instance of the GamePointers struct.
GamePointers AddressManager::s_pointers;

// A helper function to resolve RIP-relative addresses (like in LEA, MOV, and CALL instructions)
uintptr_t ResolveRelativeAddress(uintptr_t instructionAddress, size_t instructionSize) {
    if (!instructionAddress || instructionSize < AddressingConstants::RELATIVE_OFFSET_SIZE) return 0;
    // The relative offset is read from the last 4 bytes of the instruction.
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(instructionAddress + (instructionSize - AddressingConstants::RELATIVE_OFFSET_SIZE));
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
        LOG_ERROR("[AddressManager] AgentViewContext pattern not found.");
        s_pointers.agentArray = 0;
        return;
    }

    uintptr_t avContextFuncAddr = *avContextFuncOpt;
    LOG_INFO("[AddressManager] Found AgentViewContext at: 0x%p", (void*)avContextFuncAddr);

    std::optional<uintptr_t> leaInstructionOpt = kx::PatternScanner::FindPattern(std::string(kx::AGENT_ARRAY_LEA_PATTERN), avContextFuncAddr, AddressingConstants::AGENT_ARRAY_SEARCH_RANGE);

    if (!leaInstructionOpt) {
        LOG_ERROR("[AddressManager] Could not find AgentArray LEA instruction inside AvContext.");
        s_pointers.agentArray = 0;
        return;
    }

    uintptr_t leaInstructionAddress = *leaInstructionOpt;
    int32_t relativeOffset = *reinterpret_cast<int32_t*>(leaInstructionAddress + AddressingConstants::LEA_OFFSET_POSITION);
    uintptr_t addressOfNextInstruction = leaInstructionAddress + AddressingConstants::LEA_INSTRUCTION_SIZE;
    uintptr_t agentStructBase = addressOfNextInstruction + relativeOffset;
    s_pointers.agentArray = agentStructBase + AddressingConstants::AGENT_ARRAY_OFFSET;

    LOG_INFO("[AddressManager] -> SUCCESS: AgentArray resolved to: 0x%p", (void*)s_pointers.agentArray);
}

void AddressManager::ScanWorldViewContextPtr() {
    std::optional<uintptr_t> landmarkOpt = kx::PatternScanner::FindPattern(
        std::string(kx::WORLD_VIEW_CONTEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!landmarkOpt) {
        LOG_ERROR("[AddressManager] WorldViewContext pattern not found.");
        s_pointers.worldViewContextPtr = 0;
        return;
    }

    uintptr_t landmarkAddress = *landmarkOpt;
    uintptr_t movInstructionAddr = landmarkAddress - AddressingConstants::MOV_INSTRUCTION_SIZE;

    int32_t relativeOffset = *reinterpret_cast<int32_t*>(movInstructionAddr + AddressingConstants::MOV_OFFSET_POSITION);
    uintptr_t addressOfNextInstruction = movInstructionAddr + AddressingConstants::MOV_INSTRUCTION_SIZE;
    uintptr_t staticPointerAddress = addressOfNextInstruction + relativeOffset;

    s_pointers.worldViewContextPtr = *reinterpret_cast<uintptr_t*>(staticPointerAddress);

    if (s_pointers.worldViewContextPtr) {
        LOG_INFO("[AddressManager] -> SUCCESS: WorldViewContext resolved to: 0x%p", (void*)s_pointers.worldViewContextPtr);
    }
    else {
        LOG_ERROR("[AddressManager] ERROR: WvContext static address was null.");
    }
}

void AddressManager::ScanBgfxContextFunc()
{
    std::optional<uintptr_t> getContextOpt = kx::PatternScanner::FindPattern(
        std::string(kx::BGFX_CONTEXT_FUNC_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!getContextOpt) {
        LOG_ERROR("[AddressManager] BGFX Context function pattern not found.");
        s_pointers.bgfxContextFunc = 0;
        return;
    }

    // The pattern is located inside the function. We must subtract the offset
    // to get the address of the function's first instruction.
    // Start of function: 00b41ef0
    // Start of pattern:  00b41f25
    // Offset = 0x35
    uintptr_t patternAddress = *getContextOpt;
    s_pointers.bgfxContextFunc = patternAddress - AddressingConstants::BGFX_PATTERN_OFFSET;

    LOG_INFO("[AddressManager] -> SUCCESS: BGFX Context function resolved to: 0x%p", (void*)s_pointers.bgfxContextFunc);
}

void AddressManager::ScanContextCollectionFunc()
{
    std::optional<uintptr_t> funcOpt = kx::PatternScanner::FindPattern(
        std::string(kx::CONTEXT_COLLECTION_FUNC_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!funcOpt) {
        LOG_ERROR("[AddressManager] ContextCollection function pattern not found.");
        s_pointers.contextCollectionFunc = 0;
        return;
    }

    s_pointers.contextCollectionFunc = *funcOpt;
    LOG_INFO("[AddressManager] -> SUCCESS: ContextCollection function resolved to: 0x%p", (void*)s_pointers.contextCollectionFunc);
}

void AddressManager::ScanGameThreadUpdateFunc() {
    std::optional<uintptr_t> locatorOpt = kx::PatternScanner::FindPattern(
        std::string(kx::ALERT_CONTEXT_LOCATOR_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!locatorOpt) {
        LOG_ERROR("[AddressManager] AlertContext locator pattern not found.");
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    uintptr_t callToGetterAddr = *locatorOpt - AddressingConstants::ALERT_CONTEXT_CALL_OFFSET;

    uintptr_t getterFuncAddr = ResolveRelativeAddress(callToGetterAddr, AddressingConstants::CALL_INSTRUCTION_SIZE);
    if (!getterFuncAddr) {
        s_pointers.gameThreadUpdateFunc = 0;
        return;
    }

    uintptr_t staticPtrAddr = ResolveRelativeAddress(getterFuncAddr, AddressingConstants::MOV_INSTRUCTION_SIZE);
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

    s_pointers.gameThreadUpdateFunc = vtable[AddressingConstants::GAME_THREAD_UPDATE_VTABLE_INDEX];

    LOG_INFO("[AddressManager] -> SUCCESS: GameThreadUpdate function resolved to: 0x%p", (void*)s_pointers.gameThreadUpdateFunc);
}

void AddressManager::ScanModuleInformation() {
    HMODULE hModule = GetModuleHandleA("Gw2-64.exe");
    if (!hModule) {
        LOG_ERROR("[AddressManager] Failed to get handle for Gw2-64.exe");
        return;
    }

    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        LOG_ERROR("[AddressManager] Failed to get module information for Gw2-64.exe");
        return;
    }

    s_pointers.moduleBase = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    s_pointers.moduleSize = moduleInfo.SizeOfImage;

    LOG_INFO("[AddressManager] Module Information - Base: 0x%p, Size: 0x%Ix", (void*)s_pointers.moduleBase, s_pointers.moduleSize);
}

void AddressManager::Scan() {
    LOG_INFO("[AddressManager] Scanning for memory addresses...");
    ScanModuleInformation();
    ScanContextCollectionFunc();
    ScanGameThreadUpdateFunc();
    ScanDecodeTextFunc();

    // currently unused
    //ScanAgentArray();
    //ScanWorldViewContextPtr();
    //ScanBgfxContextFunc();
}

void AddressManager::ScanDecodeTextFunc() {
    if (kx::DECODE_TEXT_PATTERN.empty()) {
        LOG_WARN("[AddressManager] DecodeText pattern is empty. Name resolution for NPCs/Objects will fail.");
        s_pointers.decodeTextFunc = 0;
        return;
    }

    std::optional<uintptr_t> patternMatch = kx::PatternScanner::FindPattern(
        std::string(kx::DECODE_TEXT_PATTERN),
        std::string(kx::TARGET_PROCESS_NAME)
    );

    if (!patternMatch) {
        LOG_ERROR("[AddressManager] DecodeText pattern not found. Name resolution for NPCs/Objects will fail.");
        s_pointers.decodeTextFunc = 0;
        return;
    }

    // The signature is assumed to start at the function entry point.
    s_pointers.decodeTextFunc = *patternMatch - 16;
    LOG_INFO("[AddressManager] -> SUCCESS: DecodeText function resolved to: 0x%p", (void*)s_pointers.decodeTextFunc);
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
