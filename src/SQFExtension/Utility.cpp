#include "SQFExtensions.hpp"
#include <intercept.hpp>
using namespace intercept::client;
using namespace intercept;
using namespace SQFExtensions;

game_value getNumberWithDef(game_value_parameter right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_number(right_arg[0]);
    return right_arg[1];
}

game_value getTextWithDef(game_value_parameter right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_text(right_arg[0]);
    return right_arg[1];
}
game_value getArrayWithDef(game_value_parameter right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_array(right_arg[0]);
    return right_arg[1];
}
game_value getBoolWithDef(game_value_parameter right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_number(right_arg[0]) > 0.f;
    if (sqf::is_text(right_arg[0]))
        return r_string(sqf::get_text(right_arg[0])).compare_case_insensitive("true");

    return right_arg[1];
}

game_value getAnyWithDef(game_value_parameter right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_number(right_arg[0]);
    if (sqf::is_text(right_arg[0]))
        return sqf::get_text(right_arg[0]);
    if (sqf::is_array(right_arg[0]))
        return sqf::get_array(right_arg[0]);
    return right_arg[1];
}

game_value selectLast(game_value_parameter right_arg) {
    return right_arg.to_array().back();
}

game_value popEnd(game_value_parameter right_arg) {
    auto elem = right_arg.to_array().back();
    right_arg.to_array().erase(right_arg.to_array().end() - 1);
    return elem;
}

game_value popFront(game_value_parameter right_arg) {
    auto elem = right_arg.to_array().front();
    right_arg.to_array().erase(right_arg.to_array().begin());
    return elem;
}

game_value pushFront(game_value_parameter left_arg, game_value_parameter right_arg) {
    auto& arr = left_arg.to_array();
    arr.emplace(arr.begin(), right_arg);
    return {};
}

game_value pushFrontUnique(game_value_parameter left_arg, game_value_parameter right_arg) {
    auto& arr = left_arg.to_array();
    if (arr.find(right_arg) == arr.end()) {
        arr.emplace(arr.begin(), right_arg);
        return true;
    }
    return false;
}


game_value findCaseInsensitive(game_value_parameter left_arg, game_value_parameter right_arg) {
    bool searchIsString = right_arg.type() == game_data_string::type_def;
    auto& arr = left_arg.to_array();
    for (int it = 0; it < left_arg.size(); it++) {
        auto& element = arr[it];
        if (searchIsString && element.data && element.type() == game_data_string::type_def) {
            if (static_cast<r_string>(element).compare_case_insensitive(static_cast<r_string>(right_arg).c_str())) return it;
        } else {
            if (element == right_arg) return it;
        }
    }
    return -1;
}

game_value inArrayCaseInsensitive(game_value_parameter right_arg, game_value_parameter left_arg) {
    bool searchIsString = right_arg.type() == game_data_string::type_def;
    auto& arr = left_arg.to_array();
    for (int it = 0; it < left_arg.size(); it++) {
        auto& element = arr[it];
        if (searchIsString && element.data && element.type() == game_data_string::type_def) {
            if (static_cast<r_string>(element).compare_case_insensitive(static_cast<r_string>(right_arg).c_str())) return true;
        } else {
            if (element == right_arg) return true;
        }
    }
    return false;
}

game_value stringStartsWith(game_value_parameter left_arg, game_value_parameter right_arg) {
    auto leftStr = static_cast<sqf_string>(left_arg);
    auto rightStr = static_cast<sqf_string>(right_arg);
    if (rightStr.size() > leftStr.size()) return false;
    return (strncmp(leftStr.c_str(), rightStr.c_str(), std::min(leftStr.size(), rightStr.size())) == 0);
}

game_value stringStartsWithCI(game_value_parameter left_arg, game_value_parameter right_arg) {
    auto leftStr = static_cast<sqf_string>(left_arg);
    auto rightStr = static_cast<sqf_string>(right_arg);
    if (rightStr.size() > leftStr.size()) return false;
    return (_strnicmp(leftStr.c_str(), rightStr.c_str(), std::min(leftStr.size(), rightStr.size())) == 0);
}


game_value arrayUnion(game_value_parameter left_arg, game_value_parameter right_arg) {
    auto& leftArr = left_arg.to_array();
    auto& rightArr = right_arg.to_array();
    auto_array<game_value> output(leftArr);
    for (auto& elem : rightArr) {
        if (output.find(elem) != leftArr.end()) leftArr.emplace_back(elem);
    }
    return output;
}


game_value regexReplace(game_value_parameter left_arg, game_value_parameter right_arg) {
    if (right_arg.size() != 2) return "";
    std::regex regr((std::string)right_arg[0]);
    return std::regex_replace((std::string)left_arg, regr, (std::string)right_arg[1]);
}

game_value getObjectConfigFromObj(game_value_parameter obj) {
    auto type = sqf::type_of(obj);

    for (std::string_view cls : { "CfgVehicles"sv, "CfgAmmo"sv, "CfgNonAIVehicles"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}

game_value getObjectConfigFromStr(game_value_parameter className) {
    sqf_string type = className;
    for (auto& cls : { "CfgVehicles"sv, "CfgAmmo"sv, "CfgNonAIVehicles"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}

game_value getItemConfigFromObj(game_value_parameter obj) {
    auto type = sqf::type_of(obj);

    for (auto& cls : { "CfgWeapons"sv, "CfgMagazines"sv, "CfgGlasses"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}

game_value getItemConfigFromStr(game_value_parameter className) {
    sqf_string type = className;
    for (auto& cls : { "CfgWeapons"sv, "CfgMagazines"sv, "CfgGlasses"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}


//https://github.com/CBATeam/CBA_A3/blob/master/addons/common/fnc_turretPath.sqf
game_value turretPath(game_value_parameter unit) {
    auto vehicle = sqf::vehicle(unit);

    for (auto& turret : sqf::all_turrets(vehicle, true)) {
        if (sqf::turret_unit(vehicle, turret) == unit) return turret;
    }
    return { auto_array<game_value>() }; //empty array
}


game_value aliveGroup(game_value_parameter grp) {
    for (auto& unit : sqf::units(static_cast<group>(grp)))
        if (sqf::alive(unit)) return true;
    return false;
}

game_value unarySpawn(game_value_parameter code) {
    return static_cast<game_value>(sqf::spawn({}, code));
}

game_value hasItem(game_value_parameter obj, game_value_parameter classn) {
    r_string classname = classn;
    auto containsString = [&classname](const sqf_return_string_list& list) {
        for (auto& item : list)
            if (item == classname) return true;
        return false;
    };

    if (containsString(sqf::get_item_cargo(sqf::uniform_container(obj)).types)) return true;
    if (containsString(sqf::get_item_cargo(sqf::vest_container(obj)).types)) return true;
    if (containsString(sqf::assigned_items(obj))) return true;

    if (sqf::goggles(obj) == classname) return true;
    if (sqf::headgear(obj) == classname) return true;
    if (sqf::hmd(obj) == classname) return true;

    if (containsString(sqf::get_item_cargo(sqf::backpack_container(obj)).types)) return true;

    return false;
}


game_value compareBoolNumber(game_value_parameter left, game_value_parameter right) {
    return static_cast<bool>(left) == (static_cast<float>(right) != 0.f);
}
game_value compareNumberBool(game_value_parameter left, game_value_parameter right) {
    return static_cast<bool>(right) == (static_cast<float>(left) != 0.f);
}



class GameInstructionSetLVar : public game_instruction {
public:
    r_string vName;
    game_value val;
    bool exec(types::__internal::game_state* state, void* t) override {
        auto sv = client::host::functions.get_engine_allocator()->setvar_func;
        sv(vName.c_str(), val);
        return false;
    }
    int stack_size(void* t) const override { return 0; };
    r_string get_name() const override { return "GameInstructionSetLVar"sv; };
    ~GameInstructionSetLVar() override {};

};

game_value FastForEach(uintptr_t, game_value_parameter left, game_value_parameter right) {
    auto& arr = left.to_array();
    auto bodyCode = static_cast<game_data_code*>(right.data.get());


    //Insert instruction to set _x
    ref<GameInstructionSetLVar> curElInstruction = rv_allocator<GameInstructionSetLVar>::create_single();
    curElInstruction->vName = "_x";
    auto oldInstructions = bodyCode->instructions;
    ref<compact_array<ref<game_instruction>>> newInstr = compact_array<ref<game_instruction>>::create(*oldInstructions, oldInstructions->size() + 1);

    std::_Copy_no_deprecate(oldInstructions->data() + 1, oldInstructions->data() + oldInstructions->size(), newInstr->data() + 2);
    newInstr->data()[1] = curElInstruction;

    bodyCode->instructions = newInstr;

    for (auto& element : arr) {
        curElInstruction->val.data = element.data;//set _x value
        sqf::call(right);
    }

    bodyCode->instructions = oldInstructions;

    return {};
}

void Utility::preStart() {

    static auto _getNumberWithDef = host::registerFunction("getNumber", "", userFunctionWrapper<getNumberWithDef>, GameDataType::SCALAR, GameDataType::ARRAY);
    static auto _getTextWithDef = host::registerFunction("getText", "", userFunctionWrapper<getTextWithDef>, GameDataType::STRING, GameDataType::ARRAY);
    static auto _getArrayWithDef = host::registerFunction("getArray", "", userFunctionWrapper<getArrayWithDef>, GameDataType::ARRAY, GameDataType::ARRAY);
    static auto _getBoolWithDef = host::registerFunction("getBool", "", userFunctionWrapper<getBoolWithDef>, GameDataType::BOOL, GameDataType::ARRAY);
    static auto _getAnyWithDef = host::registerFunction("getAny", "", userFunctionWrapper<getAnyWithDef>, GameDataType::ANY, GameDataType::ARRAY);
    static auto _selectLast = host::registerFunction("selectLast", "", userFunctionWrapper<selectLast>, GameDataType::ANY, GameDataType::ARRAY);
    static auto _popEnd = host::registerFunction("popEnd", "", userFunctionWrapper<popEnd>, GameDataType::ANY, GameDataType::ARRAY);
    static auto _popFront = host::registerFunction("popFront", "", userFunctionWrapper<popFront>, GameDataType::ANY, GameDataType::ARRAY);
    static auto _pushFront = host::registerFunction("pushFront", "", userFunctionWrapper<pushFront>, GameDataType::NOTHING, GameDataType::ARRAY, GameDataType::ANY);
    static auto _pushFrontUnique = host::registerFunction("pushFrontUnique", "", userFunctionWrapper<pushFrontUnique>, GameDataType::BOOL, GameDataType::ARRAY, GameDataType::ANY);
    static auto _findCI = host::registerFunction("findCI", "", userFunctionWrapper<findCaseInsensitive>, GameDataType::ANY, GameDataType::ARRAY, GameDataType::ANY);
    static auto _inArrayCI = host::registerFunction("inCI", "", userFunctionWrapper<inArrayCaseInsensitive>, GameDataType::ANY, GameDataType::ANY, GameDataType::ARRAY);
    static auto _startsWith = host::registerFunction("startsWith", "", userFunctionWrapper<stringStartsWith>, GameDataType::BOOL, GameDataType::STRING, GameDataType::STRING);
    static auto _startsWithCI = host::registerFunction("startsWithCI", "", userFunctionWrapper<stringStartsWith>, GameDataType::BOOL, GameDataType::STRING, GameDataType::STRING);
    static auto _arrayUnion = host::registerFunction("arrayUnion", "", userFunctionWrapper<arrayUnion>, GameDataType::ARRAY, GameDataType::ARRAY, GameDataType::ARRAY);
    static auto _regexReplace = host::registerFunction("regexReplace", "", userFunctionWrapper<regexReplace>, GameDataType::STRING, GameDataType::STRING, GameDataType::ARRAY);
    static auto _getObjectConfigFromObj = host::registerFunction("getObjectConfig", "", userFunctionWrapper<getObjectConfigFromObj>, GameDataType::CONFIG, GameDataType::OBJECT);
    static auto _getObjectConfigFromStr = host::registerFunction("getObjectConfig", "", userFunctionWrapper<getObjectConfigFromStr>, GameDataType::CONFIG, GameDataType::STRING);
    static auto _getItemConfigFromObj = host::registerFunction("getItemConfig", "", userFunctionWrapper<getObjectConfigFromObj>, GameDataType::CONFIG, GameDataType::OBJECT);
    static auto _getItemConfigFromStr = host::registerFunction("getItemConfig", "", userFunctionWrapper<getObjectConfigFromStr>, GameDataType::CONFIG, GameDataType::STRING);
    static auto _alive = host::registerFunction("alive", "", userFunctionWrapper<aliveGroup>, GameDataType::BOOL, GameDataType::GROUP);
    static auto _unarySpawn = host::registerFunction("spawn", "", userFunctionWrapper<unarySpawn>, GameDataType::SCRIPT, GameDataType::CODE);
    static auto _turretPath = host::registerFunction("turretPath", "", userFunctionWrapper<turretPath>, GameDataType::ARRAY, GameDataType::OBJECT);
    static auto _hasItem = host::registerFunction("hasItem", "", userFunctionWrapper<hasItem>, GameDataType::BOOL, GameDataType::OBJECT, GameDataType::STRING);
    static auto _cmpBoolNumber = host::registerFunction("==", "", userFunctionWrapper<compareBoolNumber>, GameDataType::BOOL, GameDataType::BOOL, GameDataType::SCALAR);
    static auto _cmpNumberBool = host::registerFunction("==", "", userFunctionWrapper<compareNumberBool>, GameDataType::BOOL, GameDataType::SCALAR, GameDataType::BOOL);
    static auto _andNumberBool = host::registerFunction("&&", "", [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
        return static_cast<bool>(right) && static_cast<float>(left) != 0.f;
    }, GameDataType::BOOL, GameDataType::SCALAR, GameDataType::BOOL);

    static auto _andBoolNumber = host::registerFunction("&&", "", [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
        return static_cast<bool>(left) && static_cast<float>(right) != 0.f;
    }, GameDataType::BOOL, GameDataType::BOOL, GameDataType::SCALAR);

    static auto _andNumberNumber = host::registerFunction("&&", "", [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
        return static_cast<float>(left) != 0.f && static_cast<float>(right) != 0.f;
    }, GameDataType::BOOL, GameDataType::SCALAR, GameDataType::SCALAR);

    static auto _currentUnit = host::registerFunction("currentUnit"sv, "Returns the current Unit (CBA_fnc_currentUnit)"sv, [](uintptr_t) -> game_value {
        auto obj = sqf::get_variable(sqf::mission_namespace(), "");
        if (obj.is_null()) return sqf::player();
        return obj;
    }, GameDataType::OBJECT);

    static auto _FastForEach = host::registerFunction(u8"FastForEach"sv, "", FastForEach, GameDataType::NOTHING, GameDataType::ARRAY, GameDataType::CODE);
    
    static auto _currentUnit = host::registerFunction("textNull"sv, "test \"\""sv, [](uintptr_t) -> game_value {
        return sqf::text("");
    }, GameDataType::TEXT);
}
