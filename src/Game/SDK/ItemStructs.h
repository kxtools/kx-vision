#pragma once

#include "../../Memory/ForeignClass.h"
#include "../../Memory/Safety.h"
#include "../GameEnums.h"

namespace kx {
	namespace ReClass {

		/**
		 * @brief Item type enumeration (based on reverse engineering)
		 * Used to identify different categories of items for slot calculation.
		 */
		enum class ItemType : uint32_t {
			Armor = 0,
			GatheringTool = 9,
		};

		/**
		 * @brief Gathering tool types (for ItemType::GatheringTool)
		 */
		enum class GatheringToolType : uint32_t {
			Sickle = 0,  // Harvesting Sickle
			Axe = 1,     // Logging Axe
			Pick = 2,    // Mining Pick
		};

		/**
		 * @brief ItemDef - Item definition with ID and rarity
		 * Contains the core properties of an item, like its ID and rarity.
		 */
		class ItemDef : public ForeignClass {
		private:
			struct Offsets {
				static constexpr uintptr_t ID = 0x28;              // uint32_t item ID
				static constexpr uintptr_t ITEM_TYPE = 0x2C;       // uint32_t item type (e.g., 9 for gathering tools)
				static constexpr uintptr_t SPECIFIC_DATA = 0x30;   // void* type-specific data block
				static constexpr uintptr_t RARITY = 0x60;          // uint32_t rarity level
				static constexpr uintptr_t TEXT_NAME_ID = 0x80;    // uint32_t text ID for the item name
			};

			/**
			 * @brief Offsets within type-specific data blocks
			 */
			struct SpecificDataOffsets {
				// For GatheringTool (ItemType = 9)
				static constexpr uintptr_t GATHERING_TOOL_TYPE = 0x18;  // uint32_t tool type (0=Sickle, 1=Axe, 2=Pick)
				static constexpr uintptr_t ARMOR_TYPE = 0x0008;
			};

		public:
			ItemDef(void* ptr) : ForeignClass(ptr) {}

			uint32_t GetId() const {
				// Reads the unique item ID (e.g., 49371 for Quiver of Swift Flight).
				// Returns 0 if the read fails.
				return ReadMemberFast<uint32_t>(Offsets::ID, 0);
			}

			Game::ItemRarity GetRarity() const {
				return ReadMemberFast<Game::ItemRarity>(Offsets::RARITY, Game::ItemRarity::None);
			}

			ItemType GetItemType() const {
				return ReadMemberFast<ItemType>(Offsets::ITEM_TYPE, static_cast<ItemType>(0));
			}

			void* GetSpecificData() const {
				return ReadMemberFast<void*>(Offsets::SPECIFIC_DATA, nullptr);
			}

			/**
			 * @brief Calculate the equipment slot ID for this item
			 * @details Reads the item type and type-specific data to determine the correct
			 *          equipment slot. This is used by the EquipItem game function.
			 * @return Equipment slot ID, or 0xFFFFFFFF if the item cannot be equipped or type is unknown
			 */
			uint32_t GetEquipmentSlotId() const {
				if (!data()) return 0;

				ItemType type = GetItemType();

				switch (type) {
				case ItemType::GatheringTool:
					return GetGatheringToolSlotId();
				case ItemType::Armor:
					return GetArmorSlotId();
				default:
					return 0;  // Unknown or non-equippable type
				}
			}

		private:
			/**
			 * @brief Get equipment slot for gathering tools
			 * @return Slot ID (0x22-0x24), or 0xFFFFFFFF if invalid
			 */
			uint32_t GetGatheringToolSlotId() const {
				// Read the specific data pointer
				uintptr_t pSpecificData = ReadMemberFast<uintptr_t>(Offsets::SPECIFIC_DATA, 0);
				if (!pSpecificData || !SafeAccess::IsMemorySafe(reinterpret_cast<void*>(pSpecificData))) {
					return 0;
				}

				// Read the tool type from the specific data block
				uint32_t toolType = 0;
				if (!SafeAccess::TryRead(
					pSpecificData + SpecificDataOffsets::GATHERING_TOOL_TYPE,
					toolType)) {
					return 0;
				}

				// Map tool type to equipment slot
				switch (static_cast<GatheringToolType>(toolType)) {
				case GatheringToolType::Sickle:
					return (uint32_t)Game::EquipmentSlot::ForagingTool;
				case GatheringToolType::Axe:
					return (uint32_t)Game::EquipmentSlot::LoggingTool;
				case GatheringToolType::Pick:
					return (uint32_t)Game::EquipmentSlot::MiningTool;
				default:
					return 0;
				}
			}

			uint32_t GetArmorSlotId() const {
				uintptr_t pSpecificData = ReadMemberFast<uintptr_t>(Offsets::SPECIFIC_DATA, 0);
				if (!pSpecificData || !SafeAccess::IsMemorySafe(reinterpret_cast<void*>(pSpecificData))) {
					return 0;
				}
				uint32_t armorType = 0;
				if (!SafeAccess::TryRead(
					pSpecificData + SpecificDataOffsets::ARMOR_TYPE,
					armorType)) {
					return 0;
				}

				switch (armorType) {
				case 0:
					return (uint32_t)Game::EquipmentSlot::Chest;
				case 2:
					return (uint32_t)Game::EquipmentSlot::Helm;
				case 3:
					return (uint32_t)Game::EquipmentSlot::Boots;
				case 5:
					return (uint32_t)Game::EquipmentSlot::Pants;
				case 6:
					return (uint32_t)Game::EquipmentSlot::Shoulders;
				case 7:
					return (uint32_t)Game::EquipmentSlot::Gloves;
				default:
					return 0;
				}
			}
		};

	} // namespace ReClass
} // namespace kx
