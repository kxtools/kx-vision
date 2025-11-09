#include "NameResolver.h"
#include "../Utils/MemorySafety.h"
#include "../Utils/StringHelpers.h"
#include "../Game/AddressManager.h"
#include <windows.h>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace kx {
    namespace NameResolver {

        // --- Asynchronous Request Management ---

        // A unique ID for each name request
        static std::atomic<uint64_t> s_nextRequestId = 1;

        // Stores the agent pointer and the resulting name for a pending request
        struct PendingRequest {
            void* agentPtr;
            std::string result;
        };

        // Thread-safe map of pending requests
        static std::unordered_map<uint64_t, PendingRequest> s_pendingRequests;
        static std::mutex s_requestsMutex;

        // --- Caching ---
        static std::unordered_map<void*, std::string> s_nameCache;
        static std::mutex s_nameCacheMutex;

        // --- Game Function Signatures ---
        typedef void* (__fastcall* GetCodedName_t)(void* this_ptr);
        typedef void(__fastcall* DecodeGameText_t)(void* codedTxt, void* callback, void* ctx);

        // The callback from the game. 'ctx' will be our request ID.
        void __fastcall DecodeNameCallback(void* ctx, wchar_t* decodedText) {
            if (!ctx || !decodedText || decodedText[0] == L'\0') {
                return;
            }

            // --- FIX: Immediately copy the temporary game buffer into a stable wstring ---
            // The 'decodedText' pointer is only guaranteed to be valid during this function call.
            // By copying it instantly, we protect against the original buffer being overwritten.
            std::wstring safeDecodedText(decodedText);

            // Now, perform the conversion using our safe, local copy.
            std::string utf8Name = StringHelpers::WCharToUTF8String(safeDecodedText.c_str());
            if (utf8Name.empty()) {
                return;
            }

            // Lock the mutex to safely update the pending request map
            std::lock_guard<std::mutex> lock(s_requestsMutex);
            uint64_t requestId = reinterpret_cast<uint64_t>(ctx);
            auto it = s_pendingRequests.find(requestId);
            if (it != s_pendingRequests.end()) {
                it->second.result = std::move(utf8Name);
            }
        }

        // Helper to get the coded name pointer
        static void* GetCodedNamePointerSEH(void* agent_ptr, uint8_t type) {
            __try {
                uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(agent_ptr);
                if (!SafeAccess::IsMemorySafe(vtable)) return nullptr;

                GetCodedName_t pGetCodedName = reinterpret_cast<GetCodedName_t>(type == 0 ? vtable[57] : vtable[8]);
                if (!SafeAccess::IsMemorySafe((void*)pGetCodedName)) return nullptr;

                return pGetCodedName(agent_ptr);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nullptr;
            }
        }

        // Helper function to isolate the __try block
        static bool CallDecodeTextSEH(DecodeGameText_t pDecodeGameText, void* pCodedName, void* callback, void* ctx) {
            __try {
                pDecodeGameText(pCodedName, callback, ctx);
                return true; // Indicate success
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return false; // Indicate failure
            }
        }

        // This function NO LONGER returns a name. It just starts the decoding process.
        void RequestNameForAgent(void* agent_ptr, uint8_t type) {
            if (!SafeAccess::IsVTablePointerValid(agent_ptr) || !AddressManager::GetContextCollectionPtr()) {
                return;
            }

            auto pDecodeGameText = reinterpret_cast<DecodeGameText_t>(AddressManager::GetDecodeTextFunc());
            if (!pDecodeGameText) {
                return;
            }

            void* pCodedName = GetCodedNamePointerSEH(agent_ptr, type);
            if (!pCodedName) {
                return;
            }

            // Generate a unique ID for this request
            uint64_t requestId = s_nextRequestId++;

            {
                // Store the agent pointer so we know who this request is for
                std::lock_guard<std::mutex> lock(s_requestsMutex);
                s_pendingRequests[requestId] = { agent_ptr, "" };
            }

            // Call the game function via our safe helper
            bool success = CallDecodeTextSEH(
                pDecodeGameText,
                pCodedName,
                reinterpret_cast<void*>(&DecodeNameCallback),
                reinterpret_cast<void*>(requestId)
            );

            // If the call failed, remove the pending request to prevent it from sitting there forever
            if (!success) {
                std::lock_guard<std::mutex> lock(s_requestsMutex);
                s_pendingRequests.erase(requestId);
            }
        }

        // This function processes completed requests and moves them to the main cache.
        void ProcessCompletedNameRequests() {
            std::vector<std::pair<void*, std::string>> completed;

            // Safely find and remove completed requests
            {
                std::lock_guard<std::mutex> lock(s_requestsMutex);
                for (auto it = s_pendingRequests.begin(); it != s_pendingRequests.end(); ) {
                    if (!it->second.result.empty()) {
                        completed.push_back({ it->second.agentPtr, std::move(it->second.result) });
                        it = s_pendingRequests.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }

            // Add completed names to the main cache
            if (!completed.empty()) {
                std::lock_guard<std::mutex> lock(s_nameCacheMutex);
                for (const auto& pair : completed) {
                    s_nameCache[pair.first] = std::move(pair.second);
                }
            }
        }

        void CacheNamesForAgents(const std::unordered_map<void*, uint8_t>& agentPointers) {
            // 1. Process any requests that were completed since the last frame
            ProcessCompletedNameRequests();

            // 2. Request names for any new agents
            for (auto [agentPtr, type] : agentPointers) {
                if (!agentPtr) continue;

                // --- FIX: Check both the main cache AND pending requests ---
                bool alreadyProcessed = false;
                {
                    // Check if it's already in the final cache
                    std::lock_guard<std::mutex> lock(s_nameCacheMutex);
                    if (s_nameCache.count(agentPtr)) {
                        alreadyProcessed = true;
                    }
                }

                if (alreadyProcessed) {
                    continue; // Skip if we have the name
                }

                {
                    // Check if a request is already pending for this agent
                    std::lock_guard<std::mutex> lock(s_requestsMutex);
                    for (const auto& pair : s_pendingRequests) {
                        if (pair.second.agentPtr == agentPtr) {
                            alreadyProcessed = true;
                            break;
                        }
                    }
                }

                if (!alreadyProcessed) {
                    RequestNameForAgent(agentPtr, type); // Only request if not cached and not pending
                }
            }
        }

        std::string GetCachedName(void* agent_ptr) {
            if (!agent_ptr) return "";

            std::lock_guard<std::mutex> lock(s_nameCacheMutex);
            auto it = s_nameCache.find(agent_ptr);
            if (it != s_nameCache.end()) {
                return it->second;
            }
            return "";
        }

        void ClearNameCache() {
            std::lock_guard<std::mutex> lock(s_nameCacheMutex);
            s_nameCache.clear();

            // Also clear any pending requests that might now be stale
            std::lock_guard<std::mutex> req_lock(s_requestsMutex);
            s_pendingRequests.clear();
        }

        // This function is no longer used by the main loop but is kept for reference.
        std::string GetNameFromAgent(void* agent_ptr) {
            // The process is now asynchronous, so we can't get the name immediately.
            // We can only request it and check the cache later.
            return GetCachedName(agent_ptr);
        }

    } // namespace NameResolver
} // namespace kx