# GW2 Unit Systems - Evidence-Based Reference

## Overview
Guild Wars 2 uses 4 distinct coordinate/unit systems internally. This document provides evidence for each system and explains our implementation choices.

## Coordinate System Background

GW2 uses dual coordinate systems:

1. **Proprietary Engine/Renderer System (Left-Handed, Z-up):**
   - X-Axis: Right, Y-Axis: Forward, Z-Axis: Up
   - Used for rendering and general game logic

2. **Havok Physics System (Right-Handed, Y-up):**
   - X-Axis: Right, Y-Axis: Up, Z-Axis: Backward (negative Z is forward)
   - Used for physics simulation

This explains why axis swapping occurs between different coordinate systems.

## Primary Units (Our Implementation)

### 1. Meters (Mumble Link) ✅
- **Source**: Mumble Link API
- **Unit**: Real meters (SI standard)
- **Evidence**: 
  - Mumble Link specification: positions in meters
  - Our testing: 1200 units = ~30.5m measured
- **Usage**: All distance calculations, scaling, fading in ScalingConstants.h
- **Why chosen**: Universal standard, real-world tactical meaning

### 2. GW2 Units (Skill Ranges) ✅
- **Source**: In-game skill tooltips
- **Unit**: Inches (1 unit = 1 inch = 0.0254 meters)
- **Evidence**:
  - Test 1: 1200 units → 30.48m calc, ~30.5m measured ✓
  - Test 2: 900 units → 22.86m calc, ~22.9m measured ✓
  - 0.0254 = international inch-to-meter standard (1959)
- **Usage**: Distance display for tactical decisions
- **Why chosen**: Matches skill tooltips, real-world equivalent

## Internal Game Systems (Reference Only)

### 3. VisualPosition/GamePosition ✅
- **Source**: Direct memory reading (`coChar.GetVisualPosition()`, `hkpSimpleShapePhantom`)
- **Unit**: GW2's internal world coordinate system
- **Evidence**: 
  - Code: `TransformGamePositionToMumble()` in EntityExtractor.cpp
  - Conversion: `gamePos / GAME_TO_MUMBLE_SCALE_FACTOR` (÷1.23 + axis swap)
  - LayoutConstants.h: `constexpr float GAME_TO_MUMBLE_SCALE_FACTOR = 1.23f`
  - Real example: Game position `210.95, -132.64, 309.4` → Mumble `171.5, 251.5, -107.8`
- **Usage**: Converting raw visual/physics positions to Mumble Link meters
- **Why not used directly**: Internal units, requires conversion + axis remapping

### 4. Navmesh/32-bit Scaled Coordinates ✅
- **Source**: Cursor teleport and navmesh systems
- **Unit**: Raw cursor/navmesh scaled coordinates
- **Evidence**:
  - Code: `GetGroundedPosition32()` in AgentStructs.h (÷32 conversion)
  - Cursor teleport code: `cursor.X /= 32; cursor.Y /= 32; cursor.Z /= -32`
  - Real example: Cursor `6750.4, -4244.48, -9900.8` → Game `210.95, -132.64, 309.4`
- **Usage**: Terrain cursor positioning, grounded navmesh tracking
- **Why not used directly**: Navmesh-only, not real-time, not real-world units

## Observed Issues

**"3x Smaller Distances"**: Some overlays show incorrect distances (e.g., 10m instead of 30m), likely from misusing internal units as meters.

## Evidence Summary

| System | Evidence Type | Conversion | Real-World |
|--------|---------------|------------|------------|
| Meters | Mumble spec + testing | 1:1 | ✅ Real meters |
| GW2 Units | Testing + standard | ×0.0254 | ✅ Real inches |
| VisualPosition/GamePosition | Our codebase + real example | ÷1.23 + axis swap | ❌ Internal |
| Navmesh/32-bit Scaled | Our codebase + real example | ÷32 | ❌ Internal |

## Why We Use Meters for All Calculations

From ScalingConstants.h:
```cpp
constexpr float PLAYER_NPC_DISTANCE_FACTOR = 150.0f;  // 150 real meters
constexpr float FADE_START_DISTANCE = 90.0f;          // 90 real meters
```

**Reasons:**
1. **Tactical Clarity**: 150m = 150 real meters in game world
2. **Universal Standard**: SI units, no confusion
3. **Direct Source**: Mumble Link provides meters
4. **Verified Accuracy**: Tested against skill ranges
5. **Future-Proof**: International standard

## Conversion Examples

**Complete Position Conversion Chain (Real Evidence):**

Starting from Mumble Link meters: `171.5, 251.5, -107.8`

1. **Meters → GW2 Units:**
   - X: 171.5 ÷ 0.0254 = 6,752 units
   - Y: 251.5 ÷ 0.0254 = 9,902 units  
   - Z: -107.8 ÷ 0.0254 = -4,244 units
   - **Result**: `6752, 9902, -4244` (GW2 units)

2. **Meters → Game Visual/Physics:**
   - X: 171.5 × 1.23 = 210.9
   - Y: 251.5 × 1.23 = 309.3 (becomes Z in game coords)
   - Z: -107.8 × 1.23 = -132.6 (becomes Y in game coords)
   - **Result**: `210.95, -132.64, 309.4` (verified match!)

3. **Game Visual → Navmesh/32-bit:**
   - X: 210.95 × 32 = 6,750.4
   - Y: -132.64 × 32 = -4,244.48
   - Z: 309.4 × -32 = -9,900.8 (sign flip)
   - **Result**: `6750.4, -4244.48, -9900.8` (verified match!)

**Verification**: All conversions match the real measured values from our testing!

## Explicit Coordinate Mappings

**Mumble from Game (verified formulas):**
- `mumble.x = game.x / 1.23`
- `mumble.y = game.z / 1.23` ← Y↔Z axis swap
- `mumble.z = game.y / 1.23` ← Y↔Z axis swap

**Cursor from Game (verified formulas):**
- `cursor.x = 32 * game.x`
- `cursor.y = 32 * game.y`
- `cursor.z = -32 * game.z` ← Sign flip on Z

**Real Example Verification:**
- Game: `(210.95, -132.64, 309.4)`
- Mumble: `(210.95/1.23, 309.4/1.23, -132.64/1.23)` = `(171.5, 251.5, -107.8)` ✓
- Cursor: `(210.95*32, -132.64*32, 309.4*-32)` = `(6750.4, -4244.48, -9900.8)` ✓

## Summary

- ✅ **4 verified unit systems** with code/testing evidence
- ✅ **Meters + GW2 Units** for real-world tactical overlay
- ✅ **Proper conversions** from internal systems
- ❌ **Reject buggy implementations** that misuse internal units

**Our implementation uses real-world units with verified conversions.**

