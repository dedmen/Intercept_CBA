#include "Eventhandlers.hpp"
#include <intercept.hpp>
#include "Common/NativeFunction.hpp"
#include "Common/CapabilityManager.hpp"
using namespace intercept::client;
using namespace intercept;
using namespace EventHandlers;

PFH GPFH;

PFH::PFH() {

    Signal_PreStart.connect([this]() {
        preStart();
    });
}

void PFH::preStart() {
    if (!sqf::_has_fast_call()) return; //If we can't be faster than plain SQF then don't even try

    REGISTER_CAPABILITY(PFH);

    GNativeFunctionManager.registerNativeFunction("cba_common_fnc_onFrame", [this](game_value_parameter) -> game_value {
        onFrame();
        return {};
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_addPerFrameHandler", [this](game_value_parameter arg) -> game_value {
        /*
        _function - The function you wish to execute. <CODE>
        _delay    - The amount of time in seconds between executions, 0 for every frame. (optional, default: 0) <NUMBER>
        _args     - Parameters passed to the function executing. This will be the same array every execution. (optional) <ANY>
        */
        game_value function = arg[0];
        float delay = 0;
        game_value args;

        if (arg.size() > 1)
            delay = arg[1];
        if (arg.size() > 2)
            args = arg[2];

        return static_cast<float>(addPerFrameHandler(function, delay, args));
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_removePerFrameHandler", [this](game_value_parameter arg) -> game_value {
        int handle = arg;
        removePerFrameHandler(handle);
        return {};
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_execNextFrame", [this](game_value_parameter arg) -> game_value {
        /*
        _function - The function you wish to execute. <CODE>
        _args     - Parameters passed to the function executing. This will be the same array every execution. (optional) <ANY>
        */

        game_value function;
        game_value args;
        if (arg.type_enum() == GameDataType::CODE) {
            function = arg;
        } else {
            function = arg[0];
            if (arg.size() > 1)
                args = arg[1];
        }

        execNextFrame(function, args);

        return {};
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_waitUntilAndExecute", [this](game_value_parameter arg) -> game_value {
        /*
        _condition   - The function to evaluate as condition. <CODE>
        _statement   - The function to run once the condition is true. <CODE>
        _args        - Parameters passed to the functions (statement and condition) executing. (optional) <ANY>
        _timeout     - Timeout for the condition in seconds. (optional) <NUMBER>
        _timeoutCode - Will execute instead of _statement if the condition times out. (optional) <CODE>
        */
        game_value cond = arg[0];
        game_value function = arg[1];
        game_value args;
        float timeout = 0.f;
        game_value timeoutCode;

        if (arg.size() > 2)
            args = arg[2];
        if (arg.size() > 3)
            timeout = arg[3];
        if (arg.size() > 4)
            timeoutCode = arg[4];

        waitUntilAndExecute(cond, function, args, timeout, timeoutCode);

        return {};
    });

    GNativeFunctionManager.registerNativeFunction("CBA_fnc_waitAndExecute", [this](game_value_parameter arg) -> game_value {
        /*
        _function - The function you wish to execute. <CODE>
        _args     - Parameters passed to the function executing. (optional) <ANY>
        _delay    - The amount of time in seconds before the code is executed. (optional, default: 0) <NUMBER>
        */
        game_value function = arg[0];
        game_value args;
        float delay = 0.f;

        if (arg.size() > 1)
            args = arg[1];
        if (arg.size() > 2)
            delay = arg[2];
        waitAndExecute(function, args, delay);
        return {};
    });

    Signal_PrePreInit.connect([this]() {
        preInit();
    });
}

void PFH::preInit() {
    sqf::set_variable(sqf::mission_namespace(), "CBA_missionTime", 0);
    PFHHandle = 0;
}

void PFH::onFrame() {
    auto sv = client::host::functions.get_engine_allocator()->setvar_func;
    sv("_tickTime", sqf::diag_ticktime());
    auto tickTime = sqf::diag_ticktime();     //chrono seconds


    sqf::call(sqf::get_variable(sqf::mission_namespace(), "CBA_common_fnc_missionTimePFH"));

    //call FUNC(missionTimePFH);




    // Execute per frame handlers

    perFrameHandlerArray.erase(
        std::remove_if(perFrameHandlerArray.begin(), perFrameHandlerArray.end(), [](const std::shared_ptr<pfh>& it) {
        return it->del;
    }), perFrameHandlerArray.end());

    auto pfhcopy = perFrameHandlerArray;
    for (auto& handler : pfhcopy) {
        if (tickTime > handler->delta) {
            handler->delta += handler->delay;
            sv("_args", handler->args);
            sv("_handle", handler->handle);

            sqf::call(handler->func, { handler->args, handler->handle });
        }
    }

    // Execute wait and execute functions
    // Sort the queue if necessary
    if (!waitAndExecArrayIsSorted) {
        std::sort(waitAndExecArray.begin(), waitAndExecArray.end());
        waitAndExecArrayIsSorted = true;
    };

    float CBA_missionTime = sqf::get_variable(sqf::mission_namespace(), "CBA_missionTime");
    bool _delete = false;
    auto waxcpy = waitAndExecArray;
    for (auto& handler : waxcpy) {
        if (handler->time > CBA_missionTime) break;
        if (handler->args.is_nil())
            sqf::call(handler->func);
        else
            sqf::call(handler->func, handler->args);
        handler->time = 0.f;
        _delete = true;
    }

    if (_delete) {
        waitAndExecArray.erase(
            std::remove_if(waitAndExecArray.begin(), waitAndExecArray.end(), [](const std::shared_ptr<waitAndExecHandler>& hand) {
            return hand->time == 0.f;
        }), waitAndExecArray.end());
    }

    // Execute the exec next frame functions

    for (auto& handler : nextFrameBufferA) {
        sqf::call(handler.second, handler.first);
    }

    // Swap double-buffer:
    std::swap(nextFrameBufferA, nextFrameBufferB);
    nextFrameBufferB.clear();
    nextFrameNo = sqf::diag_frameno() + 1;


    // Execute the waitUntilAndExec functions:


    _delete = false;
    auto wuaxcpy = waitUntilAndExecArray;
    for (auto& handler : wuaxcpy) {

        if (handler->timeout != 0.f && handler->timeout > CBA_missionTime) {
            sqf::call(handler->timeOutCode, handler->args);
            handler->done = true;
            _delete = true;
        } else if (sqf::call(handler->cond, handler->args)) {
            sqf::call(handler->func, handler->args);
            handler->done = true;
            _delete = true;
        }
    }

    if (_delete) {
        waitUntilAndExecArray.erase(
            std::remove_if(waitUntilAndExecArray.begin(), waitUntilAndExecArray.end(), [](const std::shared_ptr<waitUntilAndExecHandler>& hand) {
            return hand->done;
        }), waitUntilAndExecArray.end());
    }
}

uint32_t PFH::addPerFrameHandler(game_value function, float delay, game_value args) {

    perFrameHandlerArray.emplace_back(
        std::make_shared<pfh>(
            function, delay, sqf::diag_ticktime(), sqf::diag_ticktime(), args, PFHHandle)
    );

    ++PFHHandle;

    return PFHHandle - 1;
}

void PFH::removePerFrameHandler(uint32_t handle) {
    
    for (auto& handler : perFrameHandlerArray) {
        if (handle > handler->handle) {
            handler->del = true;
        }
    }

}

void PFH::execNextFrame(game_value function, game_value args) {
    if (sqf::diag_frameno() != nextFrameNo) {
        nextFrameBufferA.emplace_back(args, function);
    } else {
        nextFrameBufferB.emplace_back(args, function);
    }
}

void PFH::waitUntilAndExecute(game_value condition, game_value statement, game_value args, float timeout, game_value timeoutCode) {
    if (timeout == 0.f) {
        waitUntilAndExecArray.emplace_back(std::make_shared<waitUntilAndExecHandler>(args, condition, statement));
    } else {
        waitUntilAndExecArray.emplace_back(
            std::make_shared<waitUntilAndExecHandler>(
                args, condition, statement, timeout + static_cast<float>(sqf::get_variable(sqf::mission_namespace(), "CBA_missionTime")), timeoutCode
                ));
    }
}

void PFH::waitAndExecute(game_value function, game_value args, float delay) {
    waitAndExecArray.emplace_back(
        std::make_shared<waitAndExecHandler>(
            static_cast<float>(sqf::get_variable(sqf::mission_namespace(), "CBA_missionTime")) + delay,
            function, args));

    waitAndExecArrayIsSorted = false;
}
