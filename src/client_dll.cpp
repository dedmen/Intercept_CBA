#include <intercept.hpp>
#include "SQFExtension/SQFExtensions.hpp"
#include "Keybinding/InputHandler.hpp"

int intercept::api_version() {
    return 1;
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



void intercept::pre_start() {
    SQFExtensions::Utility::preStart();
    SQFExtensions::Math::preStart();
    inputHandler.preStart();

}

void  intercept::pre_init() {

}

void intercept::post_init() {

}

void intercept::mission_ended() {

}
