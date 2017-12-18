#include "NativeFunction.hpp"
#include <intercept.hpp>
#include "common.hpp"

using namespace intercept;

NativeFunctionManager GNativeFunctionManager;


static sqf_script_type GameDataNativeFunction_type;
static GameDataType GGameDataNativeFunction_GDType;

class GameDataNativeFunction : public game_data {
public:
    GameDataNativeFunction() {}
    GameDataNativeFunction(const GameDataNativeFunction& other) : name(other.name), func(other.func) {}
    GameDataNativeFunction(r_string&& name_, NativeFunctionManager::functionType&& func_) : name(name_), func(func_) {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return GameDataNativeFunction_type; }
    ~GameDataNativeFunction() override {};

    bool get_as_bool() const override { return false; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { static r_string nm("NativeFunction"sv); return nm; }
    game_data* copy() const override { return new GameDataNativeFunction(*this); } //#TODO make sure this works
    r_string to_string() const override { return r_string("NativeFunction"sv); }
    bool equals(const game_data* other) const override {
        if (other->type() == GameDataNativeFunction_type) {
            return static_cast<const GameDataNativeFunction*>(other)->name == name;
        }
        return false;
    }
    const char* type_as_string() const override { return "NativeFunction"; }
    bool is_nil() const override { return false; }
    bool can_serialize() override { return true; }

    serialization_return serialize(param_archive& ar) override {
        game_data::serialize(ar);
        ar.serialize("FunctionName"sv, name, 1);
        if (!ar._isExporting) {
            func = GNativeFunctionManager.getFunc(name);
        }
        return serialization_return::no_error;
    }

    bool get_final() const override { return true; }

    r_string name;
    NativeFunctionManager::functionType func;
};

game_data* createGameDataNativeFunction(param_archive* ar) {
    auto x = new GameDataNativeFunction();
    if (ar)
        x->serialize(*ar);
    return x;
}

game_value createNativeFunction(std::string_view name, NativeFunctionManager::functionType func) {
    return game_value(new GameDataNativeFunction(name, std::move(func)));
}

NativeFunctionManager::NativeFunctionManager() {
    Signal_PreStart.connect([this]() {
        static auto nativeFunctionType = client::host::registerType("NativeFunction"sv, "NativeFunction"sv, "NativeFunction calling directly into Intercept"sv, "NativeFunction"sv, createGameDataNativeFunction);
        GGameDataNativeFunction_GDType = nativeFunctionType.first;
        GameDataNativeFunction_type = nativeFunctionType.second;


        static auto nativeFunctionCallUnary = client::host::registerFunction("call"sv, "Native Function call"sv, [](uintptr_t,game_value_parameter right) -> game_value {
            if (right.data && right.data->type() == GameDataNativeFunction_type) {
                return static_cast<GameDataNativeFunction*>(right.data.get())->func({});
            }
            return {};
        }, GameDataType::ANY, GGameDataNativeFunction_GDType);

        static auto nativeFunctionCallBinary = client::host::registerFunction("call"sv, "Native Function call"sv, [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
             if (right.data && right.data->type() == GameDataNativeFunction_type) {
                 return static_cast<GameDataNativeFunction*>(right.data.get())->func(left);
             }
             return {};
        }, GameDataType::ANY, GameDataType::ANY, GGameDataNativeFunction_GDType);

        static auto nativeFunctionSpawnUnary = client::host::registerFunction("spawn"sv, "Native Function spawn"sv, [](uintptr_t, game_value_parameter right) -> game_value {
            if (right.data && right.data->type() == GameDataNativeFunction_type) {
                return static_cast<GameDataNativeFunction*>(right.data.get())->func({});
            }
            return {};
        }, GameDataType::ANY, GGameDataNativeFunction_GDType);

        static auto nativeFunctionSpawnBinary = client::host::registerFunction("spawn"sv, "Native Function spawn"sv, [](uintptr_t, game_value_parameter left, game_value_parameter right) -> game_value {
            if (right.data && right.data->type() == GameDataNativeFunction_type) {
                return static_cast<GameDataNativeFunction*>(right.data.get())->func(left);
            }
            return {};
        }, GameDataType::ANY, GameDataType::ANY, GGameDataNativeFunction_GDType);



        for (auto& it : registeredFunctions) {
            sqf::set_variable(sqf::ui_namespace(), it.first, createNativeFunction(it.first, it.second));
        }
    });

     Signal_PrePreInit.connect([this]() {
         for (auto& it : registeredFunctions) {
             sqf::set_variable(sqf::mission_namespace(), it.first, createNativeFunction(it.first, it.second));
         }
     });
}

NativeFunctionManager::functionType NativeFunctionManager::getFunc(const intercept::types::r_string& name) {
    auto found = std::find_if(registeredFunctions.begin(), registeredFunctions.end(), [&name](auto& it) {
        return name == it.first;
    });

    if (found != registeredFunctions.end()) {
        return registeredFunctions[name.c_str()];
    }
    //Return default empty function
    return [](game_value_parameter) -> game_value { return {}; };
}

void NativeFunctionManager::registerNativeFunction(std::string_view name, functionType func) {
    auto found = std::find_if(registeredFunctions.begin(), registeredFunctions.end(), [&name](auto& it) {
        return name == it.first;
    });

    if (found != registeredFunctions.end()) {
        throw std::invalid_argument("Function already exists");
    }
    registeredFunctions.insert({ std::move(std::string(name)), func });
}
