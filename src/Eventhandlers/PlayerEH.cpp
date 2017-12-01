#include "Eventhandlers.hpp"
#include <intercept.hpp>
using namespace intercept::client;
using namespace intercept;
using namespace EventHandlers;



game_value CBA_oldUnit;
game_value CBA_oldGroup;
game_value CBA_oldLeader;
r_string CBA_oldWeapon;
game_value CBA_oldLoadout;
sqf::rv_unit_loadout CBA_oldLoadoutNoAmmo;
game_value CBA_oldVehicle;
game_value CBA_oldTurret;
int CBA_oldVisionMode;
r_string CBA_oldCameraView;
bool CBA_oldVisibleMap;

extern game_value turretPath(game_value_parameter unit);//From SQFExtension/Utility


game_value CBA_playerEH_EachFrame() {
    static game_value_static CBA_fnc_currentUnit = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_currentUnit");
    static game_value_static CBA_fnc_localEvent = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_localEvent");

    object _player = sqf::get_variable(sqf::mission_namespace(), "bis_fnc_moduleRemoteControl_unit", sqf::player());
    if (_player != CBA_oldUnit) {
        sqf::call(CBA_fnc_localEvent, { "cba_common_unitEvent",{ _player , CBA_oldUnit } });
        CBA_oldUnit = _player;
    }

    game_value _data = sqf::get_group(_player);
    if (_data != CBA_oldGroup) {
        sqf::call(CBA_fnc_localEvent, { "cba_common_groupEvent",{ _data , CBA_oldGroup } });
        CBA_oldGroup = _data;
    }

    _data = sqf::leader(_player);
    if (_data != CBA_oldLeader) {
        sqf::call(CBA_fnc_localEvent, { "cba_common_leaderEvent",{ _data , CBA_oldLeader } });
        CBA_oldLeader = _data;
    }

    _data = sqf::current_weapon(_player);
    if (_data != CBA_oldWeapon) {
        CBA_oldWeapon = _data;
        sqf::call(CBA_fnc_localEvent, { "cba_common_weaponEvent",{ _player , _data } });
    }

    _data = sqf::get_unit_loadout(_player);
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
            sqf::call(CBA_fnc_localEvent, { "cba_common_loadoutEvent",{ _player , _data } });
        }
    }

    _data = sqf::vehicle(_player);
    if (_data != CBA_oldVehicle) {
        CBA_oldVehicle = _data;
        sqf::call(CBA_fnc_localEvent, { "cba_common_vehicleEvent",{ _player , _data } });
    }

    _data = turretPath(_player);
    if (_data != CBA_oldTurret) {
        CBA_oldTurret = _data;
        sqf::call(CBA_fnc_localEvent, { "cba_common_turretEvent",{ _player , _data } });
    }

    auto visionMode = sqf::current_vision_mode(_player);
    if (visionMode != CBA_oldVisionMode) {
        CBA_oldVisionMode = visionMode;
        sqf::call(CBA_fnc_localEvent, { "cba_common_visionModeEvent",{ _player , visionMode } });
    }

    auto camView = sqf::camera_view();
    if (camView != CBA_oldCameraView) {
        CBA_oldCameraView = camView;
        sqf::call(CBA_fnc_localEvent, { "cba_common_cameraViewEvent",{ _player , camView } });
    }
    return {};
}


void PlayerEH::preStart() {

    static auto _playerEH = intercept::client::host::registerFunction("CBA_Intercept_playerEH", "", userFunctionWrapper<CBA_playerEH_EachFrame>, GameDataType::NOTHING);

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


    //#TODO replace playerEH Script func

}
