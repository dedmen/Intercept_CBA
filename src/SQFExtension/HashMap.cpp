#include "SQFExtensions.hpp"
#include <intercept.hpp>
#include "Common/CapabilityManager.hpp"
using namespace intercept::client;
using namespace intercept;
using namespace SQFExtensions;

types::registered_sqf_function _createHashMap;
types::registered_sqf_function _hashMapSet;
types::registered_sqf_function _hashMapFind;
types::registered_sqf_function _hashMapRemove;
types::registered_sqf_function _hashMapContains;
types::registered_sqf_function _hashMapCount;
types::registered_sqf_function _hashMapSelect;
types::registered_sqf_function _hashMapSetVar;
types::registered_sqf_function _hashMapGetVarDef;
types::registered_sqf_function _hashMapGetVarStr;
types::registered_sqf_function _hashMapGetKeyList;
static sqf_script_type GameDataHashMap_type;

class GameDataHashMap : public game_data {

public:
    GameDataHashMap() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return GameDataHashMap_type; }
    ~GameDataHashMap() override {};

    bool get_as_bool() const override { return true; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("hashmap"sv); return nm; }
    game_data* copy() const override { return new GameDataHashMap(*this); } //#TODO make sure this works
    r_string to_string() const override { return r_string("hashMap"sv); }
    //virtual bool equals(const game_data*) const override; //#TODO isEqualTo on hashMaps would be quite nice I guess?
    const char* type_as_string() const override { return "hashMap"; }
    bool is_nil() const override { return false; }
    bool can_serialize() override { return true; }//Setting this to false causes a fail in scheduled and global vars

    serialization_return serialize(param_archive& ar) override {
        game_data::serialize(ar);
        size_t entryCount;
        if (ar._isExporting) entryCount = map.size();
        //ar.serialize("entryCount"sv, entryCount, 1);
        //#TODO add array serialization functions
        //ar._p1->add_array_entry()
        //if (!ar._isExporting) {
        //
        //    for (int i = 0; i < entryCount; ++i) {
        //        s
        //    }
        //
        //
        //
        //}
        return serialization_return::no_error;
    }
    std::unordered_map<game_value, game_value> map;
};

game_data* createGameDataHashMap(param_archive* ar) {
    auto x = new GameDataHashMap();
    if (ar)
        x->serialize(*ar);
    return x;
}



game_value createHashMap() {
    return game_value(new GameDataHashMap());
}

//#define CBA_HASH_LOG

game_value hashSet(game_value_parameter hashMap, game_value_parameter args) {
    if (hashMap.is_nil() || args.size() != 2) return {}; //WTF U doin u idiot?! Stop givin me that crap
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());


    map->map[args[0]] = args[1];
#ifdef CBA_HASH_LOG
    std::stringstream str;
    str << "hashSetVar" << map << " " << static_cast<r_string>(args[0]) << "=" << static_cast<r_string>(args[1]) << "\n";
    OutputDebugStringA(str.str().c_str());
#endif
    return {};
}

game_value hashFind(game_value_parameter hashMap, game_value_parameter toFind) {
    if (hashMap.is_nil()) return {};
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    auto found = map->map.find(toFind);
    if (found != map->map.end()) {
    #ifdef CBA_HASH_LOG
        std::stringstream str;
        str << "hashGetVar" << map << " " << static_cast<r_string>(toFind) << "=" << static_cast<r_string>(found->second) << "\n";
        OutputDebugStringA(str.str().c_str());
    #endif
        return found->second;
    }
    return {};
}

game_value hashRemove(game_value_parameter hashMap, game_value_parameter toRemove) {
    if (hashMap.is_nil()) return {};
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    auto found = map->map.find(toRemove);
    if (found != map->map.end()) map->map.erase(found);
    return {};
}

game_value hashContains(game_value_parameter toFind, game_value_parameter hashMap) {
    if (hashMap.is_nil()) return {};
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    auto found = map->map.find(toFind);
    if (found != map->map.end()) return true;
    return false;
}

game_value hashCount(game_value_parameter hashMap) {
    if (hashMap.is_nil()) return 0;
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    return static_cast<float>(map->map.size());
}


game_value hashSetVar(game_value_parameter hashMap, game_value_parameter args) {
    if (hashMap.is_nil() || args.size() < 2) return {}; //WTF U doin?
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());


    map->map[args[0]] = args[1];
#ifdef CBA_HASH_LOG
    std::stringstream str;
    str << "hashSetVar" << map << " " << static_cast<r_string>(args[0]) << "=" << static_cast<r_string>(args[1]) << "\n";
    OutputDebugStringA(str.str().c_str());
#endif

    return {};
}

game_value hashGetVarDef(game_value_parameter hashMap, game_value_parameter args) {
    if (hashMap.is_nil() || args.size() < 2) return {};
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    auto found = map->map.find(args[0]);
    if (found != map->map.end()) {
    #ifdef CBA_HASH_LOG
        std::stringstream str;
        str << "hashGetVarDef" << map << " " << static_cast<r_string>(args[0]) << "=" << static_cast<r_string>(found->second) << "\n";
        OutputDebugStringA(str.str().c_str());
    #endif
        return found->second;
    }
    return args[1];
}

game_value hashGetVarStr(game_value_parameter hashMap, game_value_parameter args) {
    if (hashMap.is_nil()) return {};
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    auto found = map->map.find(args);
    if (found != map->map.end()) {
    #ifdef CBA_HASH_LOG
        std::stringstream str;
        str << "hashGetVarStr" << map << " " << static_cast<r_string>(args) << "=" << static_cast<r_string>(found->second) << "\n";
        OutputDebugStringA(str.str().c_str());
    #endif
        return found->second;
    }
    return {};
}

game_value hashGetKeyList(game_value_parameter hashMap) {
    if (hashMap.is_nil()) return 0;
    auto_array<game_value> keys;
    auto map = static_cast<GameDataHashMap*>(hashMap.data.get());
    for (auto& k : map->map) {
        keys.emplace_back(k.first);
    }

    return std::move(keys);
}

void HashMap::preStart() {

    auto codeType = host::registerType("HASHMAP"sv, "hashMap"sv, "Dis is a hashmap. It hashes things."sv, "hashMap"sv, createGameDataHashMap);
    GameDataHashMap_type = codeType.second;

    _createHashMap = host::registerFunction("createHashMap", "Creates a Hashmap", userFunctionWrapper<createHashMap>, codeType.first);
    _hashMapSet = host::registerFunction("set", "Sets a value in a Hashmap", userFunctionWrapper<hashSet>, GameDataType::NOTHING, codeType.first, GameDataType::ARRAY);
    _hashMapFind = host::registerFunction("find", "Finds an element in a Hashmap", userFunctionWrapper<hashFind>, GameDataType::ANY, codeType.first, GameDataType::ANY);
    _hashMapSelect = host::registerFunction("select", "Finds an element in a Hashmap", userFunctionWrapper<hashFind>, GameDataType::ANY, codeType.first, GameDataType::ANY);
    _hashMapRemove = host::registerFunction("deleteAt", "Deletes an element in a Hashmap", userFunctionWrapper<hashRemove>, GameDataType::NOTHING, codeType.first, GameDataType::ANY);
    _hashMapContains = host::registerFunction("in", "Checks if given element is inside Hashmap", userFunctionWrapper<hashContains>, GameDataType::BOOL, GameDataType::ANY, codeType.first);
    _hashMapCount = host::registerFunction("count", "Counts number of elements inside Hashmap", userFunctionWrapper<hashCount>, GameDataType::SCALAR, codeType.first);
    _hashMapSetVar = host::registerFunction("setVariable", "", userFunctionWrapper<hashSetVar>, GameDataType::NOTHING, codeType.first, GameDataType::ARRAY);
    _hashMapGetVarDef = host::registerFunction("getVariable", "", userFunctionWrapper<hashGetVarDef>, GameDataType::ANY, codeType.first, GameDataType::ARRAY);
    _hashMapGetVarStr = host::registerFunction("getVariable", "", userFunctionWrapper<hashGetVarStr>, GameDataType::ANY, codeType.first, GameDataType::STRING);
    _hashMapGetKeyList = host::registerFunction("allVariables", "", userFunctionWrapper<hashGetKeyList>, GameDataType::ARRAY, codeType.first);
    REGISTER_CAPABILITY(HashMap);
}
