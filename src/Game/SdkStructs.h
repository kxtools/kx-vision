#pragma once

/**
 * @file SdkStructs.h
 * @brief Canonical entry point for the public game SDK
 * 
 * Keep each foreign layout human-local: offsets, pointer traversal, getters, and
 * mechanical setters belong together in the nearest domain *Structs.h file.
 * Add a new domain file only when no existing domain fits, then include it here
 * and add it to KX-Vision.vcxproj.
 * 
 * - AgentStructs.h: Agent and coordinate wrappers for character entities
 * - CharacterStructs.h: Character, health, stats, energies, and player wrappers
 * - HavokStructs.h: Havok physics engine wrapper classes
 * - GadgetStructs.h: Gadget, agent, and coordinate wrappers for world objects
 * - ItemStructs.h: Item and key-framed object wrappers
 * - StatStructs.h: Item stat layouts
 * - EquipmentStructs.h: Equipment and inventory wrappers
 * - ContextStructs.h: Context managers and the root context collection
 */

#include "SDK/AgentStructs.h"     // Must be included first (no dependencies)
#include "SDK/CharacterStructs.h" // Depends on AgentStructs
#include "SDK/HavokStructs.h"     // Havok physics wrappers (independent)
#include "SDK/GadgetStructs.h"    // Depends on HavokStructs
#include "SDK/ItemStructs.h"      // Depends on GadgetStructs (for AgKeyFramed)
#include "SDK/StatStructs.h"
#include "SDK/EquipmentStructs.h"
#include "SDK/ContextStructs.h"   // Depends on Character and Gadget structs
