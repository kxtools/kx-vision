#pragma once

/**
 * @file SdkStructs.h
 * @brief Main header that includes all ReClass structure modules
 * 
 * This file serves as the entry point for all ReClass game structure wrappers.
 * The structures have been organized into logical modules for better maintainability:
 * 
 * - AgentStructs.h: Agent and coordinate wrappers for character entities
 * - CharacterStructs.h: Character, health, stats, energies, and player wrappers
 * - HavokStructs.h: Havok physics engine wrapper classes
 * - GadgetStructs.h: Gadget, agent, and coordinate wrappers for world objects
 * - ContextStructs.h: Context managers and the root context collection
 */

#include "SDK/AgentStructs.h"     // Must be included first (no dependencies)
#include "SDK/CharacterStructs.h" // Depends on AgentStructs
#include "SDK/HavokStructs.h"     // Havok physics wrappers (independent)
#include "SDK/GadgetStructs.h"    // Depends on HavokStructs
#include "SDK/ItemStructs.h"      // Depends on GadgetStructs (for AgKeyFramed)
#include "SDK/EquipmentStructs.h"
#include "SDK/ContextStructs.h"   // Depends on Character and Gadget structs