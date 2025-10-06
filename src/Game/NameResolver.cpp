#include "NameResolver.h"
#include "../Utils/MemorySafety.h"
#include "../Utils/StringHelpers.h"
#include "../Game/AddressManager.h"
#include <windows.h>
#include <unordered_map>
#include <mutex>

namespace kx {
    namespace NameResolver {

        // Thread-safe cache for agent names
        static std::unordered_map<void*, std::string> s_nameCache;
        static std::mutex s_nameCacheMutex;

        // A POD struct to pass as context to the callback.
        struct DecodedNameContext {
            wchar_t buffer[1024];
            bool success;
        };

        // CORRECT VTable function signature: takes a 'this' pointer, returns a pointer to the coded name structure.
        typedef void* (__fastcall* GetCodedName_t)(void* this_ptr);

        // The game's text decoding function.
        typedef void(__fastcall* DecodeGameText_t)(void* codedTxt, void* callback, void* ctx);

        // The callback type that DecodeGameText expects
        typedef void(__fastcall* DecodeCallback_t)(void* ctx, wchar_t* decodedText);


        // SEH-wrapped helper to safely copy the decoded string.
        static void SafeCopyDecodedString(wchar_t* dest, size_t destSize, const wchar_t* src) {
            __try {
                if (src && src[0] != L'\0') {
                    wcsncpy_s(dest, destSize, src, _TRUNCATE);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                // Ensure buffer is null-terminated on failure.
                if (destSize > 0) {
                    dest[0] = L'\0';
                }
            }
        }

        // The callback for DecodeGameText. This itself is fine.
        static void __fastcall DecodeNameCallback(DecodedNameContext* ctx, wchar_t* decodedText) {
            if (!ctx) {
                return;
            }
            ctx->success = false;
            ctx->buffer[0] = L'\0';

            SafeCopyDecodedString(ctx->buffer, _countof(ctx->buffer), decodedText);

            if (ctx->buffer[0] != L'\0') {
                ctx->success = true;
            }
        }

        // Helper function to get the pointer to the coded name structure from the VTable.
        static void* GetCodedNamePointerSEH(void* agent_ptr) {
            __try {
                uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(agent_ptr);
                if (!SafeAccess::IsMemorySafe(vtable)) return nullptr;

                GetCodedName_t pGetCodedName = reinterpret_cast<GetCodedName_t>(vtable[0]);
                if (!SafeAccess::IsMemorySafe((void*)pGetCodedName)) return nullptr;

                // Call the function and get the pointer to the game's coded name structure
                return pGetCodedName(agent_ptr);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nullptr;
            }
        }

        // Helper function to validate the coded name structure, based on hacklib/decode.c
        static bool IsCodedNameValid(void* pCodedName) {
            __try {
                if (!pCodedName || !SafeAccess::IsMemorySafe(pCodedName, sizeof(uint16_t))) return false;

                // The decompiled code checks if the first ushort is non-zero and passes other checks
                uint16_t firstWord = *reinterpret_cast<uint16_t*>(pCodedName);
                if (firstWord == 0) return false;
                if ((firstWord & 0x7fff) <= 0xff) return false;

                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }

        // Helper function to isolate the SEH block for the DecodeText call.
        static bool DecodeNameSEH(DecodeGameText_t pDecodeGameText, void* pCodedName, DecodedNameContext* pod_ctx) {
            __try {
                // Validate the pointer before passing it to the game's function
                if (!IsCodedNameValid(pCodedName)) {
                    return false;
                }

                DecodeCallback_t callbackPtr = reinterpret_cast<DecodeCallback_t>(&DecodeNameCallback);
                pDecodeGameText(pCodedName, reinterpret_cast<void*>(callbackPtr), pod_ctx);
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }

        std::string GetNameFromAgent(void* agent_ptr) {
            if (!SafeAccess::IsVTablePointerValid(agent_ptr)) {
                return "";
            }

            if (!AddressManager::GetContextCollectionPtr()) {
                return "";
            }

            // Step 1: Get the POINTER to the coded name from the game's VTable function.
            void* pCodedName = GetCodedNamePointerSEH(agent_ptr);
            if (!pCodedName) {
                return ""; // The VTable call failed or returned null.
            }

            // Step 2: Get the DecodeText function pointer.
            auto pDecodeGameText = reinterpret_cast<DecodeGameText_t>(AddressManager::GetDecodeTextFunc());
            if (!pDecodeGameText) {
                return "";
            }

            // Step 3: Call DecodeNameSEH, passing the pointer we received from the game.
            DecodedNameContext name_ctx = {};
            if (!DecodeNameSEH(pDecodeGameText, pCodedName, &name_ctx)) {
                return ""; // Decode failed.
            }

            // Step 4: Convert the result.
            if (name_ctx.success) {
                return StringHelpers::WCharToUTF8String(name_ctx.buffer);
            }

            return "";
        }

        void CacheNamesForAgents(const std::vector<void*>& agentPointers) {
            std::unordered_map<void*, std::string> newNames;

            for (void* agentPtr : agentPointers) {
                if (!agentPtr) continue;

                std::string name = GetNameFromAgent(agentPtr);
                if (!name.empty()) {
                    newNames[agentPtr] = std::move(name);
                }
            }

            if (!newNames.empty()) {
                std::lock_guard<std::mutex> lock(s_nameCacheMutex);
                for (auto& pair : newNames) {
                    s_nameCache[pair.first] = std::move(pair.second);
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
        }

    } // namespace NameResolver
} // namespace kx