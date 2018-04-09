#include "Eventhandlers.hpp"
#include <intercept.hpp>
#include "common.hpp"
#include "Common/CapabilityManager.hpp"
#include "Common/NativeFunction.hpp"
using namespace intercept::client;
using namespace intercept;
using namespace EventHandlers;


PlayerEH GPlayerEH;


extern game_value turretPath(game_value_parameter unit);//From SQFExtension/Utility

PlayerEH::PlayerEH() {
    Signal_PreStart.connect([this]() {
        preStart();
    });
}

void PlayerEH::preStart() {
    if (!sqf::_has_fast_call()) return; //If we can't be faster than plain SQF don't try.

    REGISTER_CAPABILITY(PlayerEH);

    GNativeFunctionManager.registerNativeFunction("cba_events_fnc_playerEH_EachFrame", [this](game_value_parameter) -> game_value {
        onFrame();
        return {};
    });

    GNativeFunctionManager.registerNativeFunction("cba_events_fnc_playerEH_Map", [this](game_value_parameter data) -> game_value {
        if (CBA_oldVisibleMap != static_cast<bool>(data)) {
            CBA_oldVisibleMap = data;
            callEvent(eventType::visibleMap, data);
        }
        return {};
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_addPlayerEventHandler", [this](game_value_parameter args) -> game_value {
        if (args.size() < 2 || !sqf::has_interface()) return -1.f;

        auto type = args.get(0);
        auto func = args.get(1);
        if (!type.has_value() || !func.has_value()) return {};
        auto typeEnum = typeFromString(*type);
        if (typeEnum == eventType::invalid) return { -1 };//#TODO throw exception?

        return static_cast<float>(addPlayerEventHandler(typeEnum, *func, args.get(2).value_or(false)));
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_removePlayerEventHandler", [this](game_value_parameter args) -> game_value {
        if (args.size() < 2 || !sqf::has_interface()) return -1.f;

        auto type = args.get(0);
        auto id = args.get(1);
        if (!type.has_value() || !id.has_value()) return {};
        auto typeEnum = typeFromString(*type);
        if (typeEnum == eventType::invalid) return { -1 };//#TODO throw exception?

        removePlayerEventHandler(typeEnum, static_cast<float>(*id));
        return {};
    });

    Signal_PrePreInit.connect([this]() {
        preInit();
    });
}

void PlayerEH::preInit() {
    CBA_oldUnit = sqf::obj_null();
    CBA_oldGroup = sqf::grp_null();
    CBA_oldLeader = sqf::obj_null();
    CBA_oldWeapon = ""sv;
    CBA_oldLoadout = game_value{};
    CBA_oldLoadoutNoAmmo = game_value{};
    CBA_oldVehicle = sqf::obj_null();
    CBA_oldTurret = game_value{};
    CBA_oldVisionMode = -1;
    CBA_oldCameraView = ""sv;
    CBA_oldVisibleMap = false;
    handlers.clear();
    nextHandle = 0;

    sqf::add_mission_event_handler("Map", "call cba_events_fnc_playerEH_Map");
    sqf::add_mission_event_handler("EachFrame", "call cba_events_fnc_playerEH_EachFrame");
}

void PlayerEH::onFrame() {
    static game_value_static CBA_fnc_currentUnit = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_currentUnit");


    object _player = sqf::get_variable(sqf::mission_namespace(), "bis_fnc_moduleRemoteControl_unit", sqf::player());

    if (_player != CBA_oldUnit) {
        callEvent(eventType::unit, { _player , CBA_oldUnit });
        CBA_oldUnit = _player;
    }

    if (!handlers[eventType::group].empty()) {
        game_value _data = sqf::get_group(_player);
        if (_data != CBA_oldGroup) {
            callEvent(eventType::group, { _data , CBA_oldGroup });
            CBA_oldGroup = _data;
        }
    }

    if (!handlers[eventType::leader].empty()) {
        game_value _data = sqf::leader(_player);
        if (_data != CBA_oldLeader) {
            callEvent(eventType::leader, { _data , CBA_oldLeader });
            CBA_oldLeader = _data;
        }
    }

    if (!handlers[eventType::weapon].empty()) {
        game_value _data = sqf::current_weapon(_player);
        if (_data != CBA_oldWeapon) {
            CBA_oldWeapon = _data;
            callEvent(eventType::weapon, { _player , _data });
        }
    }

    if (!handlers[eventType::loadout].empty()) {
        game_value _data = sqf::get_unit_loadout(_player);
        if (_data != CBA_oldLoadout) {
            CBA_oldLoadout = _data;
            auto loadout = sqf::rv_unit_loadout(_data);
            // we don't want to trigger this just because your ammo counter decreased.

            //remove ammo
            loadout.primary.primary_muzzle_magazine.ammo = -1;
            loadout.primary.primary_muzzle_magazine.count = -1;
            loadout.primary.secondary_muzzle_magazine.ammo = -1;
            loadout.primary.secondary_muzzle_magazine.count = -1;
            loadout.secondary.primary_muzzle_magazine.ammo = -1;
            loadout.secondary.primary_muzzle_magazine.count = -1;
            loadout.secondary.secondary_muzzle_magazine.ammo = -1;
            loadout.secondary.secondary_muzzle_magazine.count = -1;
            loadout.handgun.primary_muzzle_magazine.ammo = -1;
            loadout.handgun.primary_muzzle_magazine.count = -1;
            loadout.handgun.secondary_muzzle_magazine.ammo = -1;
            loadout.handgun.secondary_muzzle_magazine.count = -1;

            if (!(loadout == CBA_oldLoadoutNoAmmo)) {
                CBA_oldLoadoutNoAmmo = std::move(loadout);
                callEvent(eventType::loadout, { _player , _data });
            }
        }
    }

    if (!handlers[eventType::vehicle].empty()) {
        game_value _data = sqf::vehicle(_player);
        if (_data != CBA_oldVehicle) {
            CBA_oldVehicle = _data;
            callEvent(eventType::vehicle, { _player , _data });
        }
    }

    if (!handlers[eventType::turret].empty()) {
        game_value _data = turretPath(_player);
        if (_data != CBA_oldTurret) {
            CBA_oldTurret = _data;
            callEvent(eventType::turret, { _player , _data });
        }
    }

    if (!handlers[eventType::visionMode].empty()) {
        auto visionMode = sqf::current_vision_mode(_player);
        if (visionMode != CBA_oldVisionMode) {
            CBA_oldVisionMode = visionMode;
            callEvent(eventType::visionMode, { _player , visionMode });
        }
    }
    if (!handlers[eventType::turret].empty()) {
        auto camView = sqf::camera_view();
        if (camView != CBA_oldCameraView) {
            CBA_oldCameraView = camView;
            callEvent(eventType::cameraView, { _player , camView });
        }
    }
}

void PlayerEH::removePlayerEventHandler(eventType type, uint32_t id) {
    auto& vec = handlers[type];
    vec.erase(std::remove_if(vec.begin(), vec.end(), [id](auto& it) {
        return it.first == id;
    }), vec.end());
}

uint32_t PlayerEH::addPlayerEventHandler(eventType type, game_value function, bool applyRetroactively) {

    if (applyRetroactively && !CBA_oldUnit.is_null())
        switch (type) {
            case eventType::unit:
                sqf::call(function, { CBA_oldUnit , sqf::obj_null() });
                break;
            case eventType::weapon:
                sqf::call(function, { CBA_oldUnit , sqf::current_weapon(CBA_oldUnit) });
                break;
            case eventType::loadout:
                sqf::call(function, { CBA_oldUnit , sqf::get_unit_loadout(static_cast<object>(CBA_oldUnit)) });
                break;
            case eventType::vehicle:
                sqf::call(function, { CBA_oldUnit , sqf::vehicle(CBA_oldUnit) });
                break;
            case eventType::turret:
                sqf::call(function, { CBA_oldUnit , turretPath(CBA_oldUnit) });
                break;
            case eventType::visionMode:
                sqf::call(function, { CBA_oldUnit , sqf::current_vision_mode(CBA_oldUnit) });
                break;
            case eventType::cameraView:
                sqf::call(function, { CBA_oldUnit , sqf::camera_view() });
                break;
            case eventType::visibleMap:
                sqf::call(function, { CBA_oldUnit , sqf::visible_map() });
                break;
            case eventType::group:
                sqf::call(function, { CBA_oldUnit , sqf::group_get(CBA_oldUnit) });
                break;
            case eventType::leader:
                sqf::call(function, { CBA_oldUnit , sqf::group_get(CBA_oldUnit) });
                break;
            default:;
        }
    handlers[type].emplace_back(nextHandle, std::move(function));
    ++nextHandle;
    return nextHandle;
}

void PlayerEH::callEvent(eventType type, game_value args) {
    //static game_value_static CBA_fnc_localEvent = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_localEvent");
    //#TODO call internal localEH if enabled
    //sqf::call(CBA_fnc_localEvent, { toString(type) , args });
    for (auto& it : handlers[type])
        sqf::call(it.second, args);
}

PlayerEH::eventType PlayerEH::typeFromString(const r_string& name) {
    if (name == "unit"sv) return eventType::unit;
    if (name == "weapon"sv) return eventType::weapon;
    if (name == "loadout"sv) return eventType::loadout;
    if (name == "vehicle"sv) return eventType::vehicle;
    if (name == "turret"sv) return eventType::turret;
    if (name == "visionmode"sv) return eventType::visionMode;
    if (name == "cameraview"sv) return eventType::cameraView;
    if (name == "visiblemap"sv) return eventType::visibleMap;
    if (name == "group"sv) return eventType::group;
    if (name == "leader"sv) return eventType::leader;
    return eventType::invalid;
}

std::string_view PlayerEH::toString(eventType type) {
    switch (type) {
        case eventType::unit: return "unit"sv;
        case eventType::weapon: return "weapon"sv;
        case eventType::loadout: return "loadout"sv;
        case eventType::vehicle: return "vehicle"sv;
        case eventType::turret: return "turret"sv;
        case eventType::visionMode:return "visionmode"sv;
        case eventType::cameraView: return "cameraview"sv;
        case eventType::visibleMap: return "visiblemap"sv;
        case eventType::group: return "group"sv;
        case eventType::leader: return "leader"sv;
    }
    return "invalid"sv;
}
