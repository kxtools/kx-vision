#pragma once
#include "../../Core/Settings.h"
#include "../Data/ESPEntityTypes.h"

namespace kx {

class RenderSettingsHelper {
public:
    static bool ShouldRenderBox(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.renderBox;
            case ESPEntityType::NPC:    return s.npcESP.renderBox;
            case ESPEntityType::Gadget: return s.objectESP.renderBox;
            case ESPEntityType::AttackTarget: return s.objectESP.renderBox;
            default: return false;
        }
    }

    static bool ShouldRenderWireframe(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.renderWireframe;
            case ESPEntityType::NPC:    return s.npcESP.renderWireframe;
            case ESPEntityType::Gadget: return s.objectESP.renderWireframe;
            case ESPEntityType::AttackTarget: return s.objectESP.renderWireframe;
            default: return false;
        }
    }

    static bool ShouldRenderDistance(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.renderDistance;
            case ESPEntityType::NPC:    return s.npcESP.renderDistance;
            case ESPEntityType::Gadget: return s.objectESP.renderDistance;
            case ESPEntityType::AttackTarget: return s.objectESP.renderDistance;
            default: return false;
        }
    }

    static bool ShouldRenderDot(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.renderDot;
            case ESPEntityType::NPC:    return s.npcESP.renderDot;
            case ESPEntityType::Gadget: return s.objectESP.renderDot;
            case ESPEntityType::AttackTarget: return s.objectESP.renderDot;
            default: return false;
        }
    }
    
    static bool ShouldRenderName(const Settings& s, ESPEntityType type) {
        if (type == ESPEntityType::Player) return s.playerESP.renderPlayerName;
        return false;
    }

    static bool ShouldRenderHealthPercentage(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.showHealthPercentage;
            case ESPEntityType::NPC:    return s.npcESP.showHealthPercentage;
            case ESPEntityType::Gadget: return s.objectESP.showHealthPercentage;
            case ESPEntityType::AttackTarget: return s.objectESP.showHealthPercentage;
            default: return false;
        }
    }
    
    static bool ShouldRenderGadgetSphere(const Settings& s, ESPEntityType type) {
        if (type == ESPEntityType::Gadget || type == ESPEntityType::AttackTarget) {
            return s.objectESP.renderSphere;
        }
        return false;
    }

    static bool ShouldRenderGadgetCircle(const Settings& s, ESPEntityType type) {
        if (type == ESPEntityType::Gadget || type == ESPEntityType::AttackTarget) {
            return s.objectESP.renderCircle;
        }
        return false;
    }

    static bool ShouldShowDamageNumbers(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.showDamageNumbers;
            case ESPEntityType::NPC:    return s.npcESP.showDamageNumbers;
            case ESPEntityType::Gadget: return s.objectESP.showDamageNumbers;
            case ESPEntityType::AttackTarget: return s.objectESP.showDamageNumbers;
            default: return false;
        }
    }

    static bool ShouldShowBurstDps(const Settings& s, ESPEntityType type) {
        switch (type) {
            case ESPEntityType::Player: return s.playerESP.showBurstDps;
            case ESPEntityType::NPC:    return s.npcESP.showBurstDps;
            case ESPEntityType::Gadget: return s.objectESP.showBurstDps;
            case ESPEntityType::AttackTarget: return s.objectESP.showBurstDps;
            default: return false;
        }
    }

    static GearDisplayMode GetPlayerGearDisplayMode(const Settings& s) {
        return s.playerESP.enableGearDisplay ? s.playerESP.gearDisplayMode : GearDisplayMode::Compact;
    }

    static EnergyDisplayType GetPlayerEnergyDisplayType(const Settings& s) {
        return s.playerESP.energyDisplayType;
    }
    
    static bool IsBoxAllowedForSize(const Settings& s, ESPEntityType type, float height) {
        if ((type == ESPEntityType::Gadget || type == ESPEntityType::AttackTarget) && 
             height > s.objectESP.maxBoxHeight) {
            return false;
        }
        return true;
    }
};

}

