#pragma once

/**
 * @file ReClassStructs.h
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

#include "ReClass/AgentStructs.h"     // Must be included first (no dependencies)
#include "ReClass/CharacterStructs.h" // Depends on AgentStructs
#include "ReClass/HavokStructs.h"     // Havok physics wrappers (independent)
#include "ReClass/GadgetStructs.h"    // Depends on HavokStructs
#include "ReClass/EquipmentStructs.h"
#include "ReClass/ContextStructs.h"   // Depends on Character and Gadget structs