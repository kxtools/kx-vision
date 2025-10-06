#pragma once

#include <string>
#include <vector>

namespace kx {
    namespace NameResolver {
        /**
         * @brief Retrieves the name of a generic game agent using its VTable.
         *
         * IMPORTANT: This function requires the game's TLS context to be valid.
         * It should ONLY be called from the game thread where DecodeText can safely execute.
         * For render thread usage, use GetCachedName() instead.
         *
         * @param agent_ptr A pointer to the agent's instance in memory.
         * @return The decoded name as a std::string, or an empty string if retrieval fails.
         */
        std::string GetNameFromAgent(void* agent_ptr);

        /**
         * @brief Resolves and caches names for a batch of agent pointers.
         *
         * This function should be called from the GAME THREAD (e.g., in DetourGameThread)
         * where the TLS context is valid. It will resolve names for all provided agents
         * and store them in the cache for safe access from other threads.
         *
         * @param agentPointers Vector of agent pointers to resolve names for
         */
        void CacheNamesForAgents(const std::vector<void*>& agentPointers);

        /**
         * @brief Retrieves a cached name for an agent pointer.
         *
         * This function is THREAD-SAFE and can be called from any thread (e.g., render thread).
         * It returns the cached name if available, or an empty string if not found.
         *
         * @param agent_ptr The agent pointer to look up
         * @return The cached name, or empty string if not found
         */
        std::string GetCachedName(void* agent_ptr);

        /**
         * @brief Clears old entries from the name cache.
         *
         * Should be called periodically to prevent the cache from growing indefinitely
         * as agents are destroyed and new ones are created.
         */
        void ClearNameCache();

    } // namespace NameResolver
} // namespace kx
