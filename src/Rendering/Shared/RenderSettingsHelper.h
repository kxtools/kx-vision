#pragma once
#include "../../Core/Settings.h"
#include "../Data/EntityTypes.h"

namespace kx {

class RenderSettingsHelper {
public:
    static bool IsObjectType(EntityTypes type) {
        return type == EntityTypes::Gadget || 
               type == EntityTypes::AttackTarget || 
               type == EntityTypes::Item;
    }

    static bool ShouldRenderBox(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.renderBox;
            case EntityTypes::NPC:    return s.npcESP.renderBox;
            case EntityTypes::Gadget: return s.objectESP.renderBox;
            case EntityTypes::AttackTarget: return s.objectESP.renderBox;
            case EntityTypes::Item: return s.objectESP.renderBox;
            default: return false;
        }
    }

    static bool ShouldRenderWireframe(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.renderWireframe;
            case EntityTypes::NPC:    return s.npcESP.renderWireframe;
            case EntityTypes::Gadget: return s.objectESP.renderWireframe;
            case EntityTypes::AttackTarget: return s.objectESP.renderWireframe;
            case EntityTypes::Item: return s.objectESP.renderWireframe;
            default: return false;
        }
    }

    static bool ShouldRenderDistance(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.renderDistance;
            case EntityTypes::NPC:    return s.npcESP.renderDistance;
            case EntityTypes::Gadget: return s.objectESP.renderDistance;
            case EntityTypes::AttackTarget: return s.objectESP.renderDistance;
            case EntityTypes::Item: return s.objectESP.renderDistance;
            default: return false;
        }
    }

    static bool ShouldRenderDot(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.renderDot;
            case EntityTypes::NPC:    return s.npcESP.renderDot;
            case EntityTypes::Gadget: return s.objectESP.renderDot;
            case EntityTypes::AttackTarget: return s.objectESP.renderDot;
            case EntityTypes::Item: return s.objectESP.renderDot;
            default: return false;
        }
    }
    
    static bool ShouldRenderName(const Settings& s, EntityTypes type) {
        if (type == EntityTypes::Player) return s.playerESP.renderPlayerName;
        return false;
    }

    static bool ShouldRenderHealthPercentage(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.showHealthPercentage;
            case EntityTypes::NPC:    return s.npcESP.showHealthPercentage;
            case EntityTypes::Gadget: return s.objectESP.showHealthPercentage;
            case EntityTypes::AttackTarget: return s.objectESP.showHealthPercentage;
            case EntityTypes::Item: return s.objectESP.showHealthPercentage;
            default: return false;
        }
    }
    
    static bool ShouldRenderGadgetSphere(const Settings& s, EntityTypes type) {
        if (IsObjectType(type)) {
            return s.objectESP.renderSphere;
        }
        return false;
    }

    static bool ShouldRenderGadgetCircle(const Settings& s, EntityTypes type) {
        if (IsObjectType(type)) {
            return s.objectESP.renderCircle;
        }
        return false;
    }

    static bool ShouldShowDamageNumbers(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.showDamageNumbers;
            case EntityTypes::NPC:    return s.npcESP.showDamageNumbers;
            case EntityTypes::Gadget: return s.objectESP.showDamageNumbers;
            case EntityTypes::AttackTarget: return s.objectESP.showDamageNumbers;
            case EntityTypes::Item: return s.objectESP.showDamageNumbers;
            default: return false;
        }
    }

    static bool ShouldShowBurstDps(const Settings& s, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return s.playerESP.showBurstDps;
            case EntityTypes::NPC:    return s.npcESP.showBurstDps;
            case EntityTypes::Gadget: return s.objectESP.showBurstDps;
            case EntityTypes::AttackTarget: return s.objectESP.showBurstDps;
            case EntityTypes::Item: return s.objectESP.showBurstDps;
            default: return false;
        }
    }

    static GearDisplayMode GetPlayerGearDisplayMode(const Settings& s) {
        return s.playerESP.enableGearDisplay ? s.playerESP.gearDisplayMode : GearDisplayMode::Compact;
    }

    static EnergyDisplayType GetPlayerEnergyDisplayType(const Settings& s) {
        return s.playerESP.energyDisplayType;
    }
    
    static bool IsBoxAllowedForSize(const Settings& s, EntityTypes type, float height) {
        if (IsObjectType(type) && height > s.objectESP.maxBoxHeight) {
            return false;
        }
        return true;
    }
};

}

