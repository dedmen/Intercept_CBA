#include "Eventhandlers.hpp"
#include "Common/NativeFunction.hpp"
#include <intercept.hpp>

using namespace EventHandlers;
Events GEvents;

Events::Events() {
    GNativeFunctionManager.registerNativeFunction("MyTestFunctin", [](game_value_parameter) -> game_value {
        return 123;
    });
}

