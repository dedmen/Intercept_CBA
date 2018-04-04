#include "SQFExtensions.hpp"
#include <intercept.hpp>
#include <pointers.hpp>
#include <future>
#include "Common/CapabilityManager.hpp"
#include <csignal>
using namespace intercept::client;
using namespace intercept;
using namespace SQFExtensions;
using SQFPar = game_value_parameter;

static sqf_script_type GameDataElseIf_type;

class GameDataElseIf : public game_data {

public:
    GameDataElseIf() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return GameDataElseIf_type; }
    ~GameDataElseIf() override {}

    bool get_as_bool() const override { return false; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("elseIfType"sv); return nm; }
    game_data* copy() const override { return new GameDataElseIf(*this); } //#TODO make sure this works
    r_string to_string() const override { return r_string("elseIfType"sv); }
    //virtual bool equals(const game_data*) const override; //#TODO ?
    const char* type_as_string() const override { return "elseIfType"; }
    bool is_nil() const override { return false; }
    bool can_serialize() override { return true; }//Setting this to false causes a fail in scheduled and global vars

    serialization_return serialize(param_archive& ar) override {
        game_data::serialize(ar); //BBLAAAAAAHHHH //#TODO implement? maybe?

        return serialization_return::no_error;
    }
    struct statement {
        std::variant<bool, game_value> condition;
        bool checkCondition() {
            if (code.is_nil()) return false; //We might not have code if user does 'if this then that elseif this;'
            if (condition.index() == 0)
                return std::get<bool>(condition);
            return sqf::call(std::get<game_value>(condition));
        }
        game_value code;
    };
    game_value baseCode; //The code from `if COND then CODE elseif othercode...` triggered if the if condition matches
    std::vector<statement> statements;
};

game_data* createGameDataElseIf(param_archive* ar) {
    auto x = new GameDataElseIf();
    if (ar)
        x->serialize(*ar);
    return x;
}

game_value createElseIf() {
    return game_value(new GameDataElseIf());
}



game_value getNumberWithDef(SQFPar right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_number(right_arg[0]);
    return right_arg[1];
}

game_value getTextWithDef(SQFPar right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_text(right_arg[0]);
    return right_arg[1];
}
game_value getArrayWithDef(SQFPar right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_array(right_arg[0]);
    return right_arg[1];
}
game_value getBoolWithDef(SQFPar right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_number(right_arg[0]) > 0.f;
    if (sqf::is_text(right_arg[0]))
        return r_string(sqf::get_text(right_arg[0])).compare_case_insensitive("true");

    return right_arg[1];
}

game_value getAnyWithDef(SQFPar right_arg) {
    if (right_arg.size() != 2) return {};
    if (sqf::is_number(right_arg[0]))
        return sqf::get_number(right_arg[0]);
    if (sqf::is_text(right_arg[0]))
        return sqf::get_text(right_arg[0]);
    if (sqf::is_array(right_arg[0]))
        return sqf::get_array(right_arg[0]);
    return right_arg[1];
}

game_value selectLast(SQFPar right_arg) {
    return right_arg.to_array().back();
}

game_value popEnd(SQFPar right_arg) {
    auto elem = right_arg.to_array().back();
    right_arg.to_array().erase(right_arg.to_array().end() - 1);
    return elem;
}

game_value popFront(SQFPar right_arg) {
    auto elem = right_arg.to_array().front();
    right_arg.to_array().erase(right_arg.to_array().begin());
    return elem;
}

game_value pushFront(SQFPar left_arg, SQFPar right_arg) {
    auto& arr = left_arg.to_array();
    arr.emplace(arr.begin(), right_arg);
    return {};
}

game_value pushFrontUnique(SQFPar left_arg, SQFPar right_arg) {
    auto& arr = left_arg.to_array();
    if (arr.find(right_arg) == arr.end()) {
        arr.emplace(arr.begin(), right_arg);
        return true;
    }
    return false;
}


game_value findCaseInsensitive(SQFPar left_arg, SQFPar right_arg) {
    bool searchIsString = right_arg.type() == game_data_string::type_def;
    auto& arr = left_arg.to_array();
    for (size_t it = 0; it < left_arg.size(); it++) {
        auto& element = arr[it];
        if (searchIsString && element.data && element.type() == game_data_string::type_def) {
            if (static_cast<r_string>(element).compare_case_insensitive(static_cast<r_string>(right_arg).c_str())) return it;
        } else {
            if (element == right_arg) return it;
        }
    }
    return -1;
}

game_value inArrayCaseInsensitive(SQFPar right_arg, SQFPar left_arg) {
    bool searchIsString = right_arg.type() == game_data_string::type_def;
    auto& arr = left_arg.to_array();
    for (size_t it = 0; it < left_arg.size(); it++) {
        auto& element = arr[it];
        if (searchIsString && element.data && element.type() == game_data_string::type_def) {
            if (static_cast<r_string>(element).compare_case_insensitive(static_cast<r_string>(right_arg).c_str())) return true;
        } else {
            if (element == right_arg) return true;
        }
    }
    return false;
}

game_value stringStartsWith(SQFPar left_arg, SQFPar right_arg) {
    auto leftStr = static_cast<sqf_string>(left_arg);
    auto rightStr = static_cast<sqf_string>(right_arg);
    if (rightStr.size() > leftStr.size()) return false;
    return (strncmp(leftStr.c_str(), rightStr.c_str(), std::min(leftStr.size(), rightStr.size())) == 0);
}

game_value stringStartsWithCI(SQFPar left_arg, SQFPar right_arg) {
    auto leftStr = static_cast<sqf_string>(left_arg);
    auto rightStr = static_cast<sqf_string>(right_arg);
    if (rightStr.size() > leftStr.size()) return false;
    return (_strnicmp(leftStr.c_str(), rightStr.c_str(), std::min(leftStr.size(), rightStr.size())) == 0);
}


game_value arrayUnion(SQFPar left_arg, SQFPar right_arg) {
    auto& leftArr = left_arg.to_array();
    auto& rightArr = right_arg.to_array();
    auto_array<game_value> output(leftArr);
    for (auto& elem : rightArr) {
        if (output.find(elem) != leftArr.end()) leftArr.emplace_back(elem);
    }
    return output;
}


game_value regexReplace(SQFPar left_arg, SQFPar right_arg) {
    if (right_arg.size() != 2) return "";
    std::regex regr((std::string)right_arg[0]);
    return std::regex_replace((std::string)left_arg, regr, (std::string)right_arg[1]);
}

game_value getObjectConfigFromObj(SQFPar obj) {
    auto type = sqf::type_of(obj);

    for (std::string_view cls : { "CfgVehicles"sv, "CfgAmmo"sv, "CfgNonAIVehicles"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}

game_value getObjectConfigFromStr(SQFPar className) {
    sqf_string type = className;
    for (auto& cls : { "CfgVehicles"sv, "CfgAmmo"sv, "CfgNonAIVehicles"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}

game_value getItemConfigFromObj(SQFPar obj) {
    auto type = sqf::type_of(obj);

    for (auto& cls : { "CfgWeapons"sv, "CfgMagazines"sv, "CfgGlasses"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}

game_value getItemConfigFromStr(SQFPar className) {
    sqf_string type = className;
    for (auto& cls : { "CfgWeapons"sv, "CfgMagazines"sv, "CfgGlasses"sv }) {
        auto cfgClass = sqf::config_entry() >> cls >> type;
        if (sqf::is_class(cfgClass)) return static_cast<game_value>(cfgClass);

    }
    return static_cast<game_value>(sqf::config_null());
}


//https://github.com/CBATeam/CBA_A3/blob/master/addons/common/fnc_turretPath.sqf
game_value turretPath(SQFPar unit) {
    auto vehicle = sqf::vehicle(unit);

    for (auto& turret : sqf::all_turrets(vehicle, true)) {
        if (sqf::turret_unit(vehicle, turret) == unit) return turret;
    }
    return { auto_array<game_value>() }; //empty array
}


game_value aliveGroup(SQFPar grp) {
    for (auto& unit : sqf::units(static_cast<group>(grp)))
        if (sqf::alive(unit)) return true;
    return false;
}

game_value unarySpawn(SQFPar code) {
    return static_cast<game_value>(sqf::spawn({}, code));
}

game_value hasItem(SQFPar obj, SQFPar classn) {
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


game_value compareBoolNumber(SQFPar left, SQFPar right) {
    return static_cast<bool>(left) == (static_cast<float>(right) != 0.f);
}
game_value compareNumberBool(SQFPar left, SQFPar right) {
    return static_cast<bool>(right) == (static_cast<float>(left) != 0.f);
}



class GameInstructionSetLVar : public game_instruction {
public:
    r_string vName;
    game_value val;
    bool exec(game_state& state, vm_context& t) override {
        auto sv = client::host::functions.get_engine_allocator()->setvar_func;
        sv(vName.c_str(), val);
        return false;
    }
    int stack_size(void* t) const override { return 0; };
    r_string get_name() const override { return "GameInstructionSetLVar"sv; };
    ~GameInstructionSetLVar() override {};

};

game_value FastForEach(uintptr_t, SQFPar left, SQFPar right) {
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

#pragma optimize( "gts", off )
#ifdef _MSC_VER
__declspec(noinline)
#endif
uintptr_t surfaceTexture_callST(SQFPar right) {
    uintptr_t stackBuf[512];//More stackpushing to make sure our async doesn't touch this 
    client::host::functions.invoke_raw_unary(__sqf::unary__surfacetype__array__ret__string, right);
    stackBuf[211] = 13;
    return reinterpret_cast<uintptr_t>(&stackBuf[511]);
}

uintptr_t surfaceTexture_TestOffs(void* armaBase,uintptr_t stackBase, uint32_t i, const char*& rawName) {

    uintptr_t* ptr = reinterpret_cast<uintptr_t*>(stackBase - i);
    MEMORY_BASIC_INFORMATION info;
    __try {
        //memory alignment check
        if (!*ptr || *ptr & 0xFF) return 0;

        if (!VirtualQuery(reinterpret_cast<void*>(*ptr), &info, sizeof(info))) return 0;
        if (!(info.AllocationProtect & PAGE_READWRITE) || info.State != 0x1000 || info.Protect & PAGE_GUARD) return 0;

        uintptr_t* possibleVtablePtr = reinterpret_cast<uintptr_t*>(*ptr);
        if (!possibleVtablePtr) return 0; //not aligned sufficiently
        if (!VirtualQuery(reinterpret_cast<void*>(*possibleVtablePtr), &info, sizeof(info))) return 0;
        if (!(info.AllocationProtect &
            (PAGE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ)
            )
            || info.State != 0x1000 || info.Protect & PAGE_GUARD
            ) return 0;

        if (*possibleVtablePtr & 0x3) return 0;
        auto dteast = *reinterpret_cast<uint32_t*>((*ptr) + sizeof(uintptr_t));
        if (dteast == 0 || dteast > 16000) return 0;//Refcount

        uintptr_t* vtable = reinterpret_cast<uintptr_t*>(*possibleVtablePtr);
        if (!vtable || *vtable & 0x3) return 0; //not aligned sufficiently
        if (!VirtualQuery(reinterpret_cast<void*>(*vtable), &info, sizeof(info))) return 0;
        if (!(info.AllocationProtect &
            (PAGE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ)
            )
            || info.State != 0x1000 || info.Protect & PAGE_GUARD
            ) return 0;
        --vtable;//vtable info ptr
        if (!VirtualQuery(reinterpret_cast<void*>(*vtable), &info, sizeof(info))) return 0;
        if (!(info.AllocationProtect &
            (PAGE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ)
            )
            || info.State != 0x1000 || info.Protect & PAGE_GUARD
            ) return 0;
        if (info.AllocationBase != armaBase) return 0;


        class v1 {
            virtual void doStuff() noexcept {}
        };
        class v2 : public v1 {
            void doStuff() noexcept override {}
        };
        v2* v = reinterpret_cast<v2*>(possibleVtablePtr);
        auto& id = typeid(*v);
        rawName = id.raw_name();
        return reinterpret_cast<uintptr_t>(possibleVtablePtr);
    }
    __except (true) {
        //__debugbreak();
    }

    return 0;
}


game_value surfaceTexture(uintptr_t, SQFPar right) {

    //#ifdef _DEBUG
    //    //Doesn't work on Debug build because of /RTC and /GS which smaaaash the stack
    //    return {}; //#TODO proper default value
    //#endif

    uintptr_t stackBase = surfaceTexture_callST(right);
    static uintptr_t offset = 0;
    uintptr_t tx = 0;
    if (offset) {
        uintptr_t* ptr = reinterpret_cast<uintptr_t*>(stackBase - offset);
        tx = *ptr;
    } else {
        //New thread to make sure the searching doesn't overwrite the stack
        auto wt = std::async(std::launch::async, [stackBase, &right]() -> uintptr_t {

            auto armaBase = GetModuleHandleA(NULL);

            for (uint32_t i = 0; i < 1024*sizeof(uintptr_t); i += sizeof(uintptr_t)) {
                
                const char* rawName = nullptr;
                uintptr_t tx = surfaceTexture_TestOffs(armaBase, stackBase, i, rawName);

                if (rawName && tx && strcmp(rawName, ".?AVTextureD3D11@DX11@@") == 0) {
                    offset = i;

                    auto name = *reinterpret_cast<r_string*>(tx + (0xC * sizeof(uintptr_t)));
                    auto surfaceInfo = *reinterpret_cast<uintptr_t*>(tx + (0xB * sizeof(uintptr_t)));

                    auto surfType = *reinterpret_cast<r_string*>(surfaceInfo + (0x4 * sizeof(uintptr_t)));
                    r_string shouldSurfType = client::host::functions.invoke_raw_unary(__sqf::unary__surfacetype__array__ret__string, right);

                    if (surfType != shouldSurfType) continue; //Sometimes there are other textures on the stack

                    return tx;
                }
            }
            return 0;
        });
        wt.wait();
        if (wt.valid())
            tx = wt.get();
    }
    offset = 0; //#TODO remove this. This disables "caching" and is baaad
    if (tx) {
        auto name = *reinterpret_cast<r_string*>(tx + (0xC * sizeof(uintptr_t)));
        return name;
    }

    return {};
}
#pragma optimize( "", on )

void Utility::preStart() {

    static auto _getNumberWithDef = host::register_sqf_command("getNumber", "", userFunctionWrapper<getNumberWithDef>, game_data_type::SCALAR, game_data_type::ARRAY);
    static auto _getTextWithDef = host::register_sqf_command("getText", "", userFunctionWrapper<getTextWithDef>, game_data_type::STRING, game_data_type::ARRAY);
    static auto _getArrayWithDef = host::register_sqf_command("getArray", "", userFunctionWrapper<getArrayWithDef>, game_data_type::ARRAY, game_data_type::ARRAY);
    static auto _getBoolWithDef = host::register_sqf_command("getBool", "", userFunctionWrapper<getBoolWithDef>, game_data_type::BOOL, game_data_type::ARRAY);
    static auto _getAnyWithDef = host::register_sqf_command("getAny", "", userFunctionWrapper<getAnyWithDef>, game_data_type::ANY, game_data_type::ARRAY);
    static auto _selectLast = host::register_sqf_command("selectLast", "", userFunctionWrapper<selectLast>, game_data_type::ANY, game_data_type::ARRAY);
    static auto _popEnd = host::register_sqf_command("popEnd", "", userFunctionWrapper<popEnd>, game_data_type::ANY, game_data_type::ARRAY);
    static auto _popFront = host::register_sqf_command("popFront", "", userFunctionWrapper<popFront>, game_data_type::ANY, game_data_type::ARRAY);
    static auto _pushFront = host::register_sqf_command("pushFront", "", userFunctionWrapper<pushFront>, game_data_type::NOTHING, game_data_type::ARRAY, game_data_type::ANY);
    static auto _pushFrontUnique = host::register_sqf_command("pushFrontUnique", "", userFunctionWrapper<pushFrontUnique>, game_data_type::BOOL, game_data_type::ARRAY, game_data_type::ANY);
    static auto _findCI = host::register_sqf_command("findCI", "", userFunctionWrapper<findCaseInsensitive>, game_data_type::ANY, game_data_type::ARRAY, game_data_type::ANY);
    static auto _inArrayCI = host::register_sqf_command("inCI", "", userFunctionWrapper<inArrayCaseInsensitive>, game_data_type::ANY, game_data_type::ANY, game_data_type::ARRAY);
    static auto _startsWith = host::register_sqf_command("startsWith", "", userFunctionWrapper<stringStartsWith>, game_data_type::BOOL, game_data_type::STRING, game_data_type::STRING);
    static auto _startsWithCI = host::register_sqf_command("startsWithCI", "", userFunctionWrapper<stringStartsWith>, game_data_type::BOOL, game_data_type::STRING, game_data_type::STRING);
    static auto _arrayUnion = host::register_sqf_command("arrayUnion", "", userFunctionWrapper<arrayUnion>, game_data_type::ARRAY, game_data_type::ARRAY, game_data_type::ARRAY);
    static auto _regexReplace = host::register_sqf_command("regexReplace", "", userFunctionWrapper<regexReplace>, game_data_type::STRING, game_data_type::STRING, game_data_type::ARRAY);
    static auto _getObjectConfigFromObj = host::register_sqf_command("getObjectConfig", "", userFunctionWrapper<getObjectConfigFromObj>, game_data_type::CONFIG, game_data_type::OBJECT);
    static auto _getObjectConfigFromStr = host::register_sqf_command("getObjectConfig", "", userFunctionWrapper<getObjectConfigFromStr>, game_data_type::CONFIG, game_data_type::STRING);
    static auto _getItemConfigFromObj = host::register_sqf_command("getItemConfig", "", userFunctionWrapper<getObjectConfigFromObj>, game_data_type::CONFIG, game_data_type::OBJECT);
    static auto _getItemConfigFromStr = host::register_sqf_command("getItemConfig", "", userFunctionWrapper<getObjectConfigFromStr>, game_data_type::CONFIG, game_data_type::STRING);
    static auto _alive = host::register_sqf_command("alive", "", userFunctionWrapper<aliveGroup>, game_data_type::BOOL, game_data_type::GROUP);
    static auto _unarySpawn = host::register_sqf_command("spawn", "", userFunctionWrapper<unarySpawn>, game_data_type::SCRIPT, game_data_type::CODE);
    static auto _turretPath = host::register_sqf_command("turretPath", "", userFunctionWrapper<turretPath>, game_data_type::ARRAY, game_data_type::OBJECT);
    static auto _hasItem = host::register_sqf_command("hasItem", "", userFunctionWrapper<hasItem>, game_data_type::BOOL, game_data_type::OBJECT, game_data_type::STRING);
    static auto _cmpBoolNumber = host::register_sqf_command("==", "", userFunctionWrapper<compareBoolNumber>, game_data_type::BOOL, game_data_type::BOOL, game_data_type::SCALAR);
    static auto _cmpNumberBool = host::register_sqf_command("==", "", userFunctionWrapper<compareNumberBool>, game_data_type::BOOL, game_data_type::SCALAR, game_data_type::BOOL);
    static auto _andNumberBool = host::register_sqf_command("&&", "", [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        return static_cast<bool>(right) && static_cast<float>(left) != 0.f;
    }, game_data_type::BOOL, game_data_type::SCALAR, game_data_type::BOOL);

    static auto _andBoolNumber = host::register_sqf_command("&&", "", [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        return static_cast<bool>(left) && static_cast<float>(right) != 0.f;
    }, game_data_type::BOOL, game_data_type::BOOL, game_data_type::SCALAR);

    static auto _andNumberNumber = host::register_sqf_command("&&", "", [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        return static_cast<float>(left) != 0.f && static_cast<float>(right) != 0.f;
    }, game_data_type::BOOL, game_data_type::SCALAR, game_data_type::SCALAR);

    static auto _currentUnit = host::register_sqf_command("currentUnit"sv, "Returns the current Unit (CBA_fnc_currentUnit)"sv, [](uintptr_t) -> game_value {
        auto obj = sqf::get_variable(sqf::mission_namespace(), "");
        if (obj.is_null()) return sqf::player();
        return obj;
    }, game_data_type::OBJECT);

    static auto _FastForEach = host::register_sqf_command(u8"FastForEach"sv, "", FastForEach, game_data_type::NOTHING, game_data_type::ARRAY, game_data_type::CODE);

    static auto _textNull = host::register_sqf_command("textNull"sv, "test \"\""sv, [](uintptr_t) -> game_value {
        return sqf::text("");
    }, game_data_type::TEXT);

    static auto _surfaceTexture = intercept::client::host::register_sqf_command("surfaceTexture"sv, "Gets the grounds surface texture at given coordinates"sv, surfaceTexture, game_data_type::STRING, game_data_type::ARRAY);
    REGISTER_CAPABILITY(surfaceTexture);


    static auto _boolThenCode = host::register_sqf_command("then"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        if (static_cast<bool>(left))
            return sqf::call(right);
        return {}; //Ret Nil
    }, game_data_type::ANY, game_data_type::BOOL, game_data_type::CODE);

    static auto _boolThenArray = host::register_sqf_command("then"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        if (static_cast<bool>(left))
            return sqf::call(right[0]);
        if (right.size() >= 2)
            return sqf::call(right[1]);
        return {}; //You passed an array with no code for the else statement? WTF dude?
    }, game_data_type::ANY, game_data_type::BOOL, game_data_type::ARRAY);


    //elseif

    auto elseIfType = host::register_sqf_type("elseIfType"sv, "elseIfType"sv, "Dis is a elseif type. It elses ifs."sv, "elseIfType"sv, createGameDataElseIf);
    GameDataElseIf_type = elseIfType.second;

    static auto _boolThenElseIf = host::register_sqf_command("then"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        bool triggerFirst = static_cast<bool>(left);
        auto elseIfT = static_cast<GameDataElseIf*>(right.data.get());

        if (triggerFirst)
            return sqf::call(elseIfT->baseCode);

        for (auto& stmt : elseIfT->statements) {
            if (stmt.checkCondition()) {
                return sqf::call(stmt.code);
            }
        }
        //else code is also in above loop with a 'true' as condition.

        return {};
    }, game_data_type::ANY, game_data_type::BOOL, elseIfType.first);
    //#TODO suppport for ifType

    static auto _elseIfCodeBool = host::register_sqf_command("elif"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        auto elseIfT = new GameDataElseIf;
        elseIfT->baseCode = left;

        elseIfT->statements.emplace_back(GameDataElseIf::statement{ static_cast<bool>(right), {} });

        return game_value(elseIfT);
    }, elseIfType.first, game_data_type::CODE, game_data_type::BOOL);


    static auto _elseIfElseIfBool = host::register_sqf_command("elif"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        auto elseIfT = static_cast<GameDataElseIf*>(left.data.get());

        elseIfT->statements.emplace_back(GameDataElseIf::statement{ static_cast<bool>(right), {} });

        return left;
    }, elseIfType.first, elseIfType.first, game_data_type::BOOL);

    static auto _elseIfCodeCode = host::register_sqf_command("elif"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        auto elseIfT = new GameDataElseIf;
        elseIfT->baseCode = left;

        elseIfT->statements.emplace_back(GameDataElseIf::statement{ right,{} });

        return game_value(elseIfT);
    }, elseIfType.first, game_data_type::CODE, game_data_type::CODE);

    static auto _elseIfElseIfCode = host::register_sqf_command("elif"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        auto elseIfT = static_cast<GameDataElseIf*>(left.data.get());

        elseIfT->statements.emplace_back(GameDataElseIf::statement{ right,{} });

        return left;
    }, elseIfType.first, elseIfType.first, game_data_type::CODE);

    static auto _elseIfThenCode = host::register_sqf_command("then"sv, "description"sv, [](uintptr_t, SQFPar left, SQFPar right) -> game_value {
        auto elseIfT = static_cast<GameDataElseIf*>(left.data.get());

        //the previous elseif planted the statement condition already
        elseIfT->statements.back().code = right;

        return left;
    }, elseIfType.first, elseIfType.first, game_data_type::CODE);


    static auto _elseIfElse = host::registerFunction("else"sv, "description"sv, [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
        auto elseIfT = static_cast<GameDataElseIf*>(left.data.get());
        elseIfT->statements.emplace_back(GameDataElseIf::statement{ true,right });
        return left;
    }, elseIfType.first, elseIfType.first, GameDataType::CODE);
    //#TODO elseif needs higher precedence
}
