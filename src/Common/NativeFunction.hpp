#pragma once
#include <shared/types.hpp>
#include <functional>

class ArmaScriptProfiler_ProfInterfaceV1;


/// Native function interface. Name: CBA_NativeFunction Version: 1
class NativeFunctionPluginInterface {
public:
    using functionType = std::function<intercept::types::game_value(intercept::types::game_value_parameter)>;
    /*
        @brief registers new Native function
        @throws std::invalid_argument if Function with that name already exists
    */
    virtual void registerNativeFunction(std::string_view name, functionType func) throw(std::invalid_argument) = 0;
};

class NativeFunctionManager {
public:
    using functionType = std::function<intercept::types::game_value(intercept::types::game_value_parameter)>;

    NativeFunctionManager();
    functionType getFunc(const intercept::types::r_string& name);
    void registerNativeFunction(std::string_view name, functionType func);
private:
    std::map<std::string, functionType> registeredFunctions;
    ArmaScriptProfiler_ProfInterfaceV1* profilerInterface{nullptr};
};

extern NativeFunctionManager GNativeFunctionManager;
