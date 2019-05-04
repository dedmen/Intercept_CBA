#include <intercept.hpp>
#include "SQFExtension/SQFExtensions.hpp"
#include "Keybinding/InputHandler.hpp"
#include "Common/CapabilityManager.hpp"

int intercept::api_version() {
    return INTERCEPT_SDK_API_VERSION;
}
InputHandler inputHandler;
void  intercept::on_frame() {
    //inputHandler.getKeyboard()->poll();
    inputHandler.fireEvents();
    //for (auto& joystick : inputHandler.getJoysticks()) {
    //    joystick->poll();
    //    std::string outp = "J" + joystick->getName() + " ";
    //    //auto& states = inputHandler.getKeyboard()->getKeyStates();
    //    auto& states = joystick->getKeyStates();
    //    uint32_t idx = 0;
    //    for (auto& it : states) {
    //        if (it) {
    //            //InputHandler::DIKToString
    //            outp += std::to_string(idx) + ", ";
    //        }
    //        idx++;
    //    }
    //    sqf::system_chat(outp);
    //}
    //
    //
    //
    //
    //
    //std::string outp = "K ";
    //auto& states2 = inputHandler.getKeyboard()->getKeyStates();
    //uint32_t idx = 0;
    //for (auto& it : states2) {
    //    if (it) {
    //        outp += InputHandler::DIKToString(idx) + ", ";
    //    }
    //    idx++;
    //}
    //sqf::system_chat(outp);

}

//Definitions for global signals
Signal<void()> Signal_PreStart;
Signal<void()> Signal_PreInit;
Signal<void()> Signal_PrePreInit;
Signal<void()> Signal_PostInit;
Signal<void()> Signal_MissionEnded;
Signal<void()> Signal_RegisterPluginInterface;


void intercept::pre_start() {
    SQFExtensions::Utility::preStart();
    SQFExtensions::Math::preStart();
    inputHandler.preStart();
    Signal_PreStart();
}

void intercept::pre_init() {
    Signal_PreInit();
}

void intercept::pre_pre_init() {
    Signal_PrePreInit();
}


void intercept::post_init() {
    Signal_PostInit();
}

void intercept::mission_ended() {
    Signal_MissionEnded();
}

void intercept::register_interfaces() {
    Signal_RegisterPluginInterface();
}