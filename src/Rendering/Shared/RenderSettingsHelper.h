#pragma once
#include "../../Core/Settings.h"
#include "../../Features/Visuals/Settings/VisualsSettings.h"
#include "../../Game/Data/EntityTypes.h"

namespace kx {

class RenderSettingsHelper {
public:
    static bool IsObjectType(EntityTypes type) {
        return type == EntityTypes::Gadget || 
               type == EntityTypes::AttackTarget || 
               type == EntityTypes::Item;
    }

    static bool ShouldRenderBox(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.renderBox;
            case EntityTypes::NPC:    return vs.npcESP.renderBox;
            case EntityTypes::Gadget: return vs.objectESP.renderBox;
            case EntityTypes::AttackTarget: return vs.objectESP.renderBox;
            case EntityTypes::Item: return vs.objectESP.renderBox;
            default: return false;
        }
    }

    static bool ShouldRenderWireframe(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.renderWireframe;
            case EntityTypes::NPC:    return vs.npcESP.renderWireframe;
            case EntityTypes::Gadget: return vs.objectESP.renderWireframe;
            case EntityTypes::AttackTarget: return vs.objectESP.renderWireframe;
            case EntityTypes::Item: return vs.objectESP.renderWireframe;
            default: return false;
        }
    }

    static bool ShouldRenderDistance(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.renderDistance;
            case EntityTypes::NPC:    return vs.npcESP.renderDistance;
            case EntityTypes::Gadget: return vs.objectESP.renderDistance;
            case EntityTypes::AttackTarget: return vs.objectESP.renderDistance;
            case EntityTypes::Item: return vs.objectESP.renderDistance;
            default: return false;
        }
    }

    static bool ShouldRenderDot(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.renderDot;
            case EntityTypes::NPC:    return vs.npcESP.renderDot;
            case EntityTypes::Gadget: return vs.objectESP.renderDot;
            case EntityTypes::AttackTarget: return vs.objectESP.renderDot;
            case EntityTypes::Item: return vs.objectESP.renderDot;
            default: return false;
        }
    }
    
    static bool ShouldRenderName(const VisualsConfiguration& vs, EntityTypes type) {
        if (type == EntityTypes::Player) return vs.playerESP.renderPlayerName;
        return false;
    }

    static bool ShouldRenderHealthPercentage(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.showHealthPercentage;
            case EntityTypes::NPC:    return vs.npcESP.showHealthPercentage;
            case EntityTypes::Gadget: return vs.objectESP.showHealthPercentage;
            case EntityTypes::AttackTarget: return vs.objectESP.showHealthPercentage;
            case EntityTypes::Item: return vs.objectESP.showHealthPercentage;
            default: return false;
        }
    }
    
    static bool ShouldRenderGadgetSphere(const VisualsConfiguration& vs, EntityTypes type) {
        if (IsObjectType(type)) {
            return vs.objectESP.renderSphere;
        }
        return false;
    }

    static bool ShouldRenderGadgetCircle(const VisualsConfiguration& vs, EntityTypes type) {
        if (IsObjectType(type)) {
            return vs.objectESP.renderCircle;
        }
        return false;
    }

    static bool ShouldShowDamageNumbers(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.showDamageNumbers;
            case EntityTypes::NPC:    return vs.npcESP.showDamageNumbers;
            case EntityTypes::Gadget: return vs.objectESP.showDamageNumbers;
            case EntityTypes::AttackTarget: return vs.objectESP.showDamageNumbers;
            case EntityTypes::Item: return vs.objectESP.showDamageNumbers;
            default: return false;
        }
    }

    static bool ShouldShowBurstDps(const VisualsConfiguration& vs, EntityTypes type) {
        switch (type) {
            case EntityTypes::Player: return vs.playerESP.showBurstDps;
            case EntityTypes::NPC:    return vs.npcESP.showBurstDps;
            case EntityTypes::Gadget: return vs.objectESP.showBurstDps;
            case EntityTypes::AttackTarget: return vs.objectESP.showBurstDps;
            case EntityTypes::Item: return vs.objectESP.showBurstDps;
            default: return false;
        }
    }

    static GearDisplayMode GetPlayerGearDisplayMode(const VisualsConfiguration& vs) {
        return vs.playerESP.enableGearDisplay ? vs.playerESP.gearDisplayMode : GearDisplayMode::Compact;
    }

    static EnergyDisplayType GetPlayerEnergyDisplayType(const VisualsConfiguration& vs) {
        return vs.playerESP.energyDisplayType;
    }
    
    static bool IsBoxAllowedForSize(const VisualsConfiguration& vs, EntityTypes type, float height) {
        if (IsObjectType(type) && height > vs.objectESP.maxBoxHeight) {
            return false;
        }
        return true;
    }
};

}

