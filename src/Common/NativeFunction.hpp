#pragma once
#include <shared/types.hpp>
#include <functional>


class NativeFunctionManager {
public:
    using functionType = std::function<intercept::types::game_value(intercept::types::game_value_parameter)>;

    NativeFunctionManager();
    functionType getFunc(const intercept::types::r_string& name);
    void registerNativeFunction(std::string_view name, functionType func);
private:
    std::map<std::string, functionType> registeredFunctions;
};

extern NativeFunctionManager GNativeFunctionManager;
