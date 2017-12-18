#pragma once
#include "common.hpp"
#include <intercept.hpp>

namespace EventHandlers {

    class PFH {
    public:
        PFH();
        using game_value = intercept::types::game_value;

        void preStart();
        void preInit();
        void onFrame();
        uint32_t addPerFrameHandler(game_value function, float delay, game_value args);
        void removePerFrameHandler(uint32_t handle);
        void execNextFrame(game_value function, game_value args);
        void waitUntilAndExecute(game_value condition, game_value statement, game_value args, float timeout, game_value timeoutCode);
        void waitAndExecute(game_value function, game_value args, float delay);







        struct pfh {
            game_value func;
            float delay;
            float delta;
            float timeAdded;
            game_value args;
            uint32_t handle;
            bool del{ false };

            pfh(game_value _1, float _2, float _3, float _4, game_value _5, uint32_t _6) {
                func = _1;
                delay = _2;
                delta = _3;
                timeAdded = _4;
                args = _5;
                handle = _6;
            }


        };
        std::vector<std::shared_ptr<pfh>> perFrameHandlerArray;
        uint32_t PFHHandle = 0;

        struct waitAndExecHandler {
            float time;
            game_value func;
            game_value args;
            waitAndExecHandler(float _1, game_value _2, game_value _3) {
                time = _1;
                func = _2;
                args = _3;
            }
            bool operator< (const waitAndExecHandler& other) const {
                return time < other.time;
            }
        };
        std::vector<std::shared_ptr<waitAndExecHandler>> waitAndExecArray;
        bool waitAndExecArrayIsSorted{0};

        //arg,func
        std::vector<std::pair<game_value, game_value>> nextFrameBufferA;
        std::vector<std::pair<game_value, game_value>> nextFrameBufferB;
        int nextFrameNo{0};

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
    };
    extern PFH GPFH;

    class PlayerEH {
    public:
        PlayerEH();

        enum class eventType {
            unit,
            weapon,
            loadout,
            vehicle,
            turret,
            visionMode,
            cameraView,
            visibleMap,
            group,
            leader,
            invalid
        };


        void preStart();
        void preInit();
        void onFrame();

        void removePlayerEventHandler(eventType type, uint32_t id);
        uint32_t addPlayerEventHandler(eventType type, game_value function, bool applyRetroactively);

        //Wrapper for CBA_fnc_localEvent
        void callEvent(eventType type, game_value args);

        game_value CBA_oldUnit;
        game_value CBA_oldGroup;
        game_value CBA_oldLeader;
        r_string CBA_oldWeapon;
        game_value CBA_oldLoadout;
        intercept::sqf::rv_unit_loadout CBA_oldLoadoutNoAmmo;
        game_value CBA_oldVehicle;
        game_value CBA_oldTurret;
        int CBA_oldVisionMode;
        r_string CBA_oldCameraView;
        bool CBA_oldVisibleMap;


        static eventType typeFromString(const r_string& name);
        static std::string_view toString(eventType type);
        std::map<eventType, std::vector<std::pair<uint32_t, game_value>>> handlers;
        uint32_t nextHandle{0};
    };
    extern PlayerEH GPlayerEH;

    class Events {
    public:
        Events();
    };
    extern Events GEvents;

}
