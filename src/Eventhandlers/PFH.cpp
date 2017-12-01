#include "Eventhandlers.hpp"
#include <intercept.hpp>
using namespace intercept::client;
using namespace intercept;
using namespace EventHandlers;

void PFH::preStart() {



}

void swapFuncs(game_value orig, game_value newCode) {
    auto c = static_cast<game_data_code*>(orig.data.get());

    auto nc = static_cast<game_data_code*>(newCode.data.get());

    auto _1 = c->code_string;
    auto _2 = c->instructions;
    auto _3 = c->_dummy1;
    auto _4 = c->_dummy;
    auto _5 = c->is_final;


    c->code_string = nc->code_string;
    c->instructions = nc->instructions;
    c->_dummy1 = nc->_dummy1;
    c->_dummy = nc->_dummy;
    c->is_final = nc->is_final;

    nc->code_string = _1;
    nc->instructions = _2;
    nc->_dummy1 = _3;
    nc->_dummy = _4;
    nc->is_final = _5;
}


struct pfh {
    game_value func;
    float delay;
    float delta;
    float timeAdded;
    game_value args;
    int handle;
    bool del{ false };

    pfh(game_value _1, float _2, float _3, float _4, game_value _5, float _6) {
        func = _1;
        delay = _2;
        delta = _3;
        timeAdded = _4;
        args = _5;
        handle = _6;
    }


};
std::vector<std::shared_ptr<pfh>> perFrameHandlerArray;
float PFHHandle = 0.f;

struct waitAndExecHandler {
    float time;
    game_value func;
    game_value args;
    waitAndExecHandler(float _1, game_value _2, game_value _3) {
        time = _1;
        func = _2;
        args = _3;
    }
    bool operator< (const waitAndExecHandler& other) {
        return time < other.time;
    }
};
std::vector<std::shared_ptr<waitAndExecHandler>> waitAndExecArray;
bool waitAndExecArrayIsSorted;

//arg,func
std::vector<std::pair<game_value, game_value>> nextFrameBufferA;
std::vector<std::pair<game_value, game_value>> nextFrameBufferB;
int nextFrameNo;

struct waitUntilAndExecHandler {
    game_value args;
    game_value cond;
    game_value func;
    waitUntilAndExecHandler(game_value _1, game_value _2, game_value _3) {
        args = _1;
        cond = _2;
        func = _3;
    }
    waitUntilAndExecHandler(game_value _1, game_value _2, game_value _3, float _4, game_value _5) {
        args = _1;
        cond = _2;
        func = _3;
        timeout = _4;
        timeOutCode = _5;
    }
    float timeout{ 0.f };
    game_value timeOutCode;
    bool done{ false };
};
std::vector<std::shared_ptr<waitUntilAndExecHandler>> waitUntilAndExecArray;


void PFH::preInit() {
    
    sqf::set_variable(sqf::mission_namespace(), "CBA_missionTime", 0);


    static auto onFrame = client::generate_custom_callback([](game_value_parameter arg) -> game_value {
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

        return {};
    });
    auto orig = sqf::get_variable(sqf::mission_namespace(), "cba_common_fnc_onFrame");
    auto newCode = sqf::compile(onFrame.first);

    swapFuncs(orig, newCode);



    static auto addPFH = client::generate_custom_callback([](game_value_parameter arg) -> game_value {
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

        perFrameHandlerArray.emplace_back(
            std::make_shared<pfh>(
                function, delay, sqf::diag_ticktime(), sqf::diag_ticktime(), args, PFHHandle)

        );

        PFHHandle += 1.f;

        return PFHHandle - 1.f;
    });
    orig = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_addPerFrameHandler");
    newCode = sqf::compile(addPFH.first);

    swapFuncs(orig, newCode);


    static auto remPFH = client::generate_custom_callback([](game_value_parameter arg) -> game_value {
        int handle = arg;
        for (auto& handler : perFrameHandlerArray) {
            if (handle > handler->handle) {
                handler->del = true;
            }
        }

        return {};
    });
    orig = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_removePerFrameHandler");
    newCode = sqf::compile(remPFH.first);

    swapFuncs(orig, newCode);






    auto pfhs = sqf::get_variable(sqf::mission_namespace(), "CBA_common_perFrameHandlerArray");

    PFHHandle = pfhs.size();

    for (auto& it : pfhs.to_array()) {
        perFrameHandlerArray.emplace_back(std::make_shared<pfh>(it[0], it[1], it[2], it[3], it[4], it[5]));
    }


    static auto nextFrame = client::generate_custom_callback([](game_value_parameter arg) -> game_value {
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





        if (sqf::diag_frameno() != nextFrameNo) {
            nextFrameBufferA.emplace_back(args, function);
        } else {
            nextFrameBufferB.emplace_back(args, function);
        }

        return {};
    });
    orig = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_execNextFrame");
    newCode = sqf::compile(nextFrame.first);

    swapFuncs(orig, newCode);


    auto fba = sqf::get_variable(sqf::mission_namespace(), "CBA_common_nextFrameBufferA");
    auto fbb = sqf::get_variable(sqf::mission_namespace(), "CBA_common_nextFrameBufferB");

    for (auto& it : fba.to_array()) {
        nextFrameBufferA.emplace_back(it[0], it[1]);
    }
    for (auto& it : fbb.to_array()) {
        nextFrameBufferB.emplace_back(it[0], it[1]);
    }



    static auto waitUntilExec = client::generate_custom_callback([](game_value_parameter arg) -> game_value {
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

        if (timeout == 0.f) {
            waitUntilAndExecArray.emplace_back(std::make_shared<waitUntilAndExecHandler>(args, cond, function));
        } else {
            waitUntilAndExecArray.emplace_back(

                std::make_shared<waitUntilAndExecHandler>(
                    args, cond, function, timeout + static_cast<float>(sqf::get_variable(sqf::mission_namespace(), "CBA_missionTime")), timeoutCode
                    ));
        }

        return {};
    });
    orig = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_waitUntilAndExecute");
    newCode = sqf::compile(waitUntilExec.first);

    swapFuncs(orig, newCode);


    auto waux = sqf::get_variable(sqf::mission_namespace(), "CBA_common_waitUntilAndExecArray");

    for (auto& it : waux.to_array()) {
        waitUntilAndExecArray.emplace_back(std::make_shared<waitUntilAndExecHandler>(it[0], it[1], it[2]));
    }




    static auto waitExec = client::generate_custom_callback([](game_value_parameter arg) -> game_value {
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

        waitAndExecArray.emplace_back(
            std::make_shared<waitAndExecHandler>(
                static_cast<float>(sqf::get_variable(sqf::mission_namespace(), "CBA_missionTime")) + delay,
                function, args));

        waitAndExecArrayIsSorted = false;

        return {};
    });
    orig = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_waitAndExecute");
    newCode = sqf::compile(waitExec.first);

    swapFuncs(orig, newCode);


    auto wax = sqf::get_variable(sqf::mission_namespace(), "CBA_common_waitAndExecArray");

    for (auto& it : wax.to_array()) {
        waitAndExecArray.emplace_back(std::make_shared<waitAndExecHandler>(it[0], it[1], it[2]));
    }
    waitAndExecArrayIsSorted = false;
}
