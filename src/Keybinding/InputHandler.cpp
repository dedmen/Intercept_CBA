#include "InputHandler.hpp"
#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"
#include <thread>
#include <string>
#include <intercept.hpp>
using namespace intercept;

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
using namespace std::chrono_literals;
static IDirectInput8 *dInput;


HWND ArmaHwnd;
DWORD mainThreadID;

BOOL CALLBACK EnumWindowsProc(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
) {


    DWORD pid{};

    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    char buf[256]{ 0 };
    GetWindowModuleFileName(hwnd, buf, 256);
    char buf2[256]{ 0 };
    GetWindowText(hwnd, buf2, 256);
    RECT r;
    GetWindowRect(hwnd, &r);
    if (r.bottom == 0 && r.left == 0) return true;

    if (tid == mainThreadID) {
        ArmaHwnd = hwnd;
        return false;
    }
    return true;

}


char * guid_to_str(const GUID& id, char * out) {
    int i;
    char * ret = out;
    out += sprintf(out, "%.8lX-%.4hX-%.4hX-", id.Data1, id.Data2, id.Data3);
    for (i = 0; i < sizeof(id.Data4); ++i) {
        out += sprintf(out, "%.2hhX", id.Data4[i]);
        if (i == 1) *(out++) = '-';
    }
    return ret;
}

BOOL CALLBACK EnumJoyStickCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
    IDirectInputDevice8 *deviceInst;

    const GUID &guid = lpddi->guidInstance;

    auto ret = dInput->CreateDevice(lpddi->guidInstance, &deviceInst, NULL);
    if (FAILED(ret)) {
        // Skip this
        return DIENUM_CONTINUE;
    }

    char guidBuf[128];
    guid_to_str(lpddi->guidInstance, guidBuf);
    auto joy = std::make_shared<Joystick>(lpddi->tszInstanceName, guidBuf, deviceInst);
    joy->initialize();
    static_cast<InputHandler*>(pvRef)->addJoystick(std::move(joy));

    return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumKeyboardCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
    IDirectInputDevice8 *deviceInst;

    const GUID &guid = lpddi->guidInstance;

    auto ret = dInput->CreateDevice(lpddi->guidInstance, &deviceInst, NULL);
    if (FAILED(ret)) {
        // Skip this
        return DIENUM_CONTINUE;
    }
    char guidBuf[128];
    guid_to_str(lpddi->guidInstance, guidBuf);
    auto keyb = std::make_shared<Keyboard>(lpddi->tszInstanceName, guidBuf, deviceInst);
    keyb->initialize();
    static_cast<InputHandler*>(pvRef)->addKeyboard(std::move(keyb));

    return DIENUM_CONTINUE;
}



struct EnumContext {

};


Keyboard::Keyboard(std::string name_, std::string guid_, IDirectInputDevice8A* device_) : name(name_), guid(guid_), device(device_) {}

void Keyboard::initialize() {
    auto ret = device->SetCooperativeLevel(ArmaHwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    ret = device->SetDataFormat(&c_dfDIKeyboard);

    //DIDEVCAPS caps{};
    //caps.dwSize = sizeof(caps);
    //ret = device->GetCapabilities(&caps);
}

static const int DIKToVK[][2] = {
    {DIK_ESCAPE,VK_ESCAPE},
    {DIK_1,'1'},
    {DIK_2,'2'},
    {DIK_3,'3'},
    {DIK_4,'4'},
    {DIK_5,'5'},
    {DIK_6,'6'},
    {DIK_7,'7'},
    {DIK_8,'8'},
    {DIK_9,'9'},
    {DIK_0,'0'},
    {DIK_MINUS,VK_OEM_MINUS},
    {DIK_EQUALS,VK_OEM_PLUS},
    {DIK_BACK,VK_BACK},
    {DIK_TAB,VK_TAB},
    {DIK_Q,'Q'},
    {DIK_W,'W'},
    {DIK_E,'E'},
    {DIK_R,'R'},
    {DIK_T,'T'},
    {DIK_Y,'Y'},
    {DIK_U,'U'},
    {DIK_I,'I'},
    {DIK_O,'O'},
    {DIK_P,'P'},
    {DIK_LBRACKET,VK_OEM_4},
    {DIK_RBRACKET,VK_OEM_6},
    {DIK_RETURN,VK_RETURN},
    {DIK_LCONTROL,VK_LCONTROL},
    {DIK_A,'A'},
    {DIK_S,'S'},
    {DIK_D,'D'},
    {DIK_F,'F'},
    {DIK_G,'G'},
    {DIK_H,'H'},
    {DIK_J,'J'},
    {DIK_K,'K'},
    {DIK_L,'L'},
    {DIK_SEMICOLON,VK_OEM_1},
    {DIK_APOSTROPHE,VK_OEM_7},
    {DIK_GRAVE,VK_OEM_3},
    {DIK_LSHIFT,VK_LSHIFT},
    {DIK_BACKSLASH,VK_OEM_5},
    {DIK_Z,'Z'},
    {DIK_X,'X'},
    {DIK_C,'C'},
    {DIK_V,'V'},
    {DIK_B,'B'},
    {DIK_N,'N'},
    {DIK_M,'M'},
    {DIK_COMMA,VK_OEM_COMMA},
    {DIK_PERIOD,VK_OEM_PERIOD},
    {DIK_SLASH,VK_OEM_2},
    {DIK_RSHIFT,VK_RSHIFT},
    {DIK_MULTIPLY,VK_MULTIPLY},
    {DIK_LMENU,VK_LMENU},
    {DIK_SPACE,VK_SPACE},
    {DIK_CAPITAL,VK_CAPITAL},
    {DIK_F1,VK_F1},
    {DIK_F2,VK_F2},
    {DIK_F3,VK_F3},
    {DIK_F4,VK_F4},
    {DIK_F5,VK_F5},
    {DIK_F6,VK_F6},
    {DIK_F7,VK_F7},
    {DIK_F8,VK_F8},
    {DIK_F9,VK_F9},
    {DIK_F10,VK_F10},
    {DIK_NUMLOCK,VK_NUMLOCK},
    {DIK_SCROLL,VK_SCROLL},
    {DIK_NUMPAD7,VK_NUMPAD7},
    {DIK_NUMPAD8,VK_NUMPAD8},
    {DIK_NUMPAD9,VK_NUMPAD9},
    {DIK_SUBTRACT,VK_SUBTRACT},
    {DIK_NUMPAD4,VK_NUMPAD4},
    {DIK_NUMPAD5,VK_NUMPAD5},
    {DIK_NUMPAD6,VK_NUMPAD6},
    {DIK_ADD,VK_ADD},
    {DIK_NUMPAD1,VK_NUMPAD1},
    {DIK_NUMPAD2,VK_NUMPAD2},
    {DIK_NUMPAD3,VK_NUMPAD3},
    {DIK_NUMPAD0,VK_NUMPAD0},
    {DIK_DECIMAL,VK_DECIMAL},
    {DIK_F11,VK_F11},
    {DIK_F12,VK_F12},
    {DIK_F13,VK_F13},
    {DIK_F14,VK_F14},
    {DIK_F15,VK_F15},
    {DIK_NUMPADENTER,VK_RETURN},
    {DIK_RCONTROL,VK_RCONTROL},
    {DIK_DIVIDE,VK_DIVIDE},
    {DIK_RMENU,VK_RMENU},
    {DIK_HOME,VK_HOME},
    {DIK_UP,VK_UP},
    {DIK_PRIOR,VK_PRIOR},
    {DIK_LEFT,VK_LEFT},
    {DIK_RIGHT,VK_RIGHT},
    {DIK_END,VK_END},
    {DIK_DOWN,VK_DOWN},
    {DIK_NEXT,VK_NEXT},
    {DIK_INSERT,VK_INSERT},
    {DIK_DELETE,VK_DELETE},
    {DIK_LWIN,VK_LWIN},
    {DIK_RWIN,VK_RWIN},
    {DIK_APPS,VK_APPS},
    {DIK_PAUSE,VK_PAUSE},
    {DIK_MUTE,VK_VOLUME_MUTE},
    {DIK_VOLUMEDOWN,VK_VOLUME_DOWN},
    {DIK_VOLUMEUP,VK_VOLUME_UP},
    {DIK_WEBHOME,VK_BROWSER_HOME},
    {DIK_WEBSEARCH,VK_BROWSER_SEARCH},
    {DIK_WEBFAVORITES,VK_BROWSER_FAVORITES},
    {DIK_WEBREFRESH,VK_BROWSER_REFRESH},
    {DIK_WEBSTOP,VK_BROWSER_STOP},
    {DIK_WEBFORWARD,VK_BROWSER_FORWARD},
    {DIK_WEBBACK,VK_BROWSER_BACK},
    {DIK_MAIL,VK_LAUNCH_MAIL},
    {DIK_MEDIASELECT,VK_LAUNCH_MEDIA_SELECT}
};

bool Keyboard::poll() {

    //if (!acquired) {
    //    //Try acquire
    //   auto  ret = device->Acquire();
    //   if (FAILED(ret)) return false;//No ACQ and can't ACQ. Game is probably out of focus
    //
    //   acquired = true; //Success. Game is back in focus now
    //}
    //
    //auto ret = device->Poll();
    ////https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectinputdevice8.idirectinputdevice8.getdevicestate(v=vs.85).aspx
    //ret = device->GetDeviceState(sizeof(keyState), keyState.data());
    //if (FAILED(ret)) {
    //    acquired = false;
    //    return false;
    //}

    for (auto& it : DIKToVK) {
        keyState[it[0]] = GetAsyncKeyState(it[1]);
    }

    return true;
}

const std::array<bool, 256>& Keyboard::getKeyStates() {
    return keyState;
}

Joystick::Joystick(std::string name_, std::string guid_, IDirectInputDevice8A* device_) : name(name_), guid(guid_), device(device_) {}

void Joystick::initialize() {
    auto ret = device->SetCooperativeLevel(ArmaHwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    ret = device->SetDataFormat(&c_dfDIJoystick);

    //DIDEVCAPS caps{};
    //caps.dwSize = sizeof(caps);
    //ret = device->GetCapabilities(&caps);
}

bool Joystick::poll() {
    if (!acquired) {
        //Try acquire
        auto  ret = device->Acquire();
        if (FAILED(ret)) return false;//No ACQ and can't ACQ. Game is probably out of focus

        acquired = true; //Success. Game is back in focus now
    }




    DIJOYSTATE  diJoyState;
    auto ret = device->Poll();
    //https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.idirectinputdevice8.idirectinputdevice8.getdevicestate(v=vs.85).aspx
    ret = device->GetDeviceState(sizeof(DIJOYSTATE), &diJoyState);
    if (FAILED(ret)) {
        acquired = false;
        return false;
    }


    povDirection = diJoyState.rgdwPOV[0]/100;
    if (diJoyState.rgdwPOV[0] == 0xFFFFFFFF) povDirection = -1;

    for (int i = 0; i < 32; ++i) {
        keyState[i] = diJoyState.rgbButtons[i];
    }
    return true;
}

const std::array<bool, 32>& Joystick::getKeyStates() { return keyState; }
intercept::types::r_string Joystick::getName() { return name; }
int16_t Joystick::getPOV() { return povDirection; }

void InputHandler::preStart() {
    mainThreadID = GetCurrentThreadId();
    EnumWindows(EnumWindowsProc, 0);

    auto handle = GetModuleHandle("intercept-cba");
    HRESULT err = DirectInput8Create(
        handle
        , DIRECTINPUT_VERSION, IID_IDirectInput8, (void **) &dInput, NULL
    );

    dInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoyStickCallback, this, DIEDFL_ATTACHEDONLY);
    dInput->EnumDevices(DI8DEVCLASS_KEYBOARD, EnumKeyboardCallback, this, DIEDFL_ATTACHEDONLY);
}

std::shared_ptr<Keyboard> InputHandler::getKeyboard() { return keyboard; }
std::vector<std::shared_ptr<Joystick>> InputHandler::getJoysticks() { return joysticks; }

void InputHandler::addKeyboard(std::shared_ptr<Keyboard> move_) {
    keyboard = std::move(move_);
}

void InputHandler::addJoystick(std::shared_ptr<Joystick> move_) {
    joysticks.emplace_back(std::move(move_));
}

void InputHandler::fireEvents() {
    std::map<std::shared_ptr<Joystick>, std::array<bool, 32>> previousButtons;
    std::map<std::shared_ptr<Joystick>, uint16_t> previousPOV;
    for (auto& j : getJoysticks()) {
        previousButtons.insert({ j, j->getKeyStates() });
        previousPOV.insert({ j, j->getPOV() });
    }
    std::map<std::shared_ptr<Joystick>, std::array<bool, 32>> nowButtons;
    std::map<std::shared_ptr<Joystick>, uint16_t> nowPOV;
    for (auto& j : getJoysticks()) {
        if (j->poll()) {
            nowButtons.insert({ j, j->getKeyStates() });
            nowPOV.insert({ j, j->getPOV() });

        }
            
    }

    struct keyEvent {
        bool pressed; //if false then released
        std::shared_ptr<Joystick> joy;
        uint32_t buttonID;
    };


    std::vector<keyEvent> events;
    auto localevt = sqf::get_variable(sqf::mission_namespace(), "CBA_fnc_localEvent");
    for (auto& it : previousButtons) {
        auto& previous = previousButtons[it.first];
        auto& previouspov = previousPOV[it.first];
        auto& nowpov = nowPOV[it.first];
        auto& nowIt = nowButtons.find(it.first);
        if (nowIt == nowButtons.end()) continue; //No poll so nothing can have changed
        auto& now = nowIt->second;


        for (uint32_t i = 0; i < previous.size(); ++i) {
             if (now[i] != previous[i]) {
                 events.emplace_back(keyEvent{ now[i], it.first, i });
             }
        }

        if (previouspov != nowpov) {
            std::string message = "pov ";
            message += it.first->getName() + "] #" + std::to_string(nowpov);
            intercept::sqf::system_chat(message);
            intercept::sqf::diag_log(message);


            sqf::call(localevt, { "CBA_Keybinds_JoyPOVChanged"sv ,{ it.first->getName() ,(int) nowpov } });


        }

    }



    for (auto& evt : events) {
        std::string message = evt.pressed ? "pressed [" : "released [";
        message += evt.joy->getName() + "] #" + std::to_string(evt.buttonID);
        intercept::sqf::system_chat(message);
        intercept::sqf::diag_log(message);

        sqf::call(localevt, { evt.pressed ? "CBA_Keybinds_JoyPressed"sv : "CBA_Keybinds_JoyReleased"sv,{ evt.joy->getName() ,(int) evt.buttonID } });

    }
}

std::string InputHandler::DIKToString(uint32_t DIKCode) {

    switch (DIKCode) {
        case DIK_ESCAPE: return "DIK_ESCAPE";
        case DIK_1: return "DIK_1";
        case DIK_2: return "DIK_2";
        case DIK_3: return "DIK_3";
        case DIK_4: return "DIK_4";
        case DIK_5: return "DIK_5";
        case DIK_6: return "DIK_6";
        case DIK_7: return "DIK_7";
        case DIK_8: return "DIK_8";
        case DIK_9: return "DIK_9";
        case DIK_0: return "DIK_0";
        case DIK_MINUS: return "DIK_MINUS";
        case DIK_EQUALS: return "DIK_EQUALS";
        case DIK_BACK: return "DIK_BACK";
        case DIK_TAB: return "DIK_TAB";
        case DIK_Q: return "DIK_Q";
        case DIK_W: return "DIK_W";
        case DIK_E: return "DIK_E";
        case DIK_R: return "DIK_R";
        case DIK_T: return "DIK_T";
        case DIK_Y: return "DIK_Y";
        case DIK_U: return "DIK_U";
        case DIK_I: return "DIK_I";
        case DIK_O: return "DIK_O";
        case DIK_P: return "DIK_P";
        case DIK_LBRACKET: return "DIK_LBRACKET";
        case DIK_RBRACKET: return "DIK_RBRACKET";
        case DIK_RETURN: return "DIK_RETURN";
        case DIK_LCONTROL: return "DIK_LCONTROL";
        case DIK_A: return "DIK_A";
        case DIK_S: return "DIK_S";
        case DIK_D: return "DIK_D";
        case DIK_F: return "DIK_F";
        case DIK_G: return "DIK_G";
        case DIK_H: return "DIK_H";
        case DIK_J: return "DIK_J";
        case DIK_K: return "DIK_K";
        case DIK_L: return "DIK_L";
        case DIK_SEMICOLON: return "DIK_SEMICOLON";
        case DIK_APOSTROPHE: return "DIK_APOSTROPHE";
        case DIK_GRAVE: return "DIK_GRAVE";
        case DIK_LSHIFT: return "DIK_LSHIFT";
        case DIK_BACKSLASH: return "DIK_BACKSLASH";
        case DIK_Z: return "DIK_Z";
        case DIK_X: return "DIK_X";
        case DIK_C: return "DIK_C";
        case DIK_V: return "DIK_V";
        case DIK_B: return "DIK_B";
        case DIK_N: return "DIK_N";
        case DIK_M: return "DIK_M";
        case DIK_COMMA: return "DIK_COMMA";
        case DIK_PERIOD: return "DIK_PERIOD";
        case DIK_SLASH: return "DIK_SLASH";
        case DIK_RSHIFT: return "DIK_RSHIFT";
        case DIK_MULTIPLY: return "DIK_MULTIPLY";
        case DIK_LMENU: return "DIK_LMENU";
        case DIK_SPACE: return "DIK_SPACE";
        case DIK_CAPITAL: return "DIK_CAPITAL";
        case DIK_F1: return "DIK_F1";
        case DIK_F2: return "DIK_F2";
        case DIK_F3: return "DIK_F3";
        case DIK_F4: return "DIK_F4";
        case DIK_F5: return "DIK_F5";
        case DIK_F6: return "DIK_F6";
        case DIK_F7: return "DIK_F7";
        case DIK_F8: return "DIK_F8";
        case DIK_F9: return "DIK_F9";
        case DIK_F10: return "DIK_F10";
        case DIK_NUMLOCK: return "DIK_NUMLOCK";
        case DIK_SCROLL: return "DIK_SCROLL";
        case DIK_NUMPAD7: return "DIK_NUMPAD7";
        case DIK_NUMPAD8: return "DIK_NUMPAD8";
        case DIK_NUMPAD9: return "DIK_NUMPAD9";
        case DIK_SUBTRACT: return "DIK_SUBTRACT";
        case DIK_NUMPAD4: return "DIK_NUMPAD4";
        case DIK_NUMPAD5: return "DIK_NUMPAD5";
        case DIK_NUMPAD6: return "DIK_NUMPAD6";
        case DIK_ADD: return "DIK_ADD";
        case DIK_NUMPAD1: return "DIK_NUMPAD1";
        case DIK_NUMPAD2: return "DIK_NUMPAD2";
        case DIK_NUMPAD3: return "DIK_NUMPAD3";
        case DIK_NUMPAD0: return "DIK_NUMPAD0";
        case DIK_DECIMAL: return "DIK_DECIMAL";
        case DIK_OEM_102: return "DIK_OEM_102";
        case DIK_F11: return "DIK_F11";
        case DIK_F12: return "DIK_F12";
        case DIK_F13: return "DIK_F13";
        case DIK_F14: return "DIK_F14";
        case DIK_F15: return "DIK_F15";
        case DIK_KANA: return "DIK_KANA";
        case DIK_ABNT_C1: return "DIK_ABNT_C1";
        case DIK_CONVERT: return "DIK_CONVERT";
        case DIK_NOCONVERT: return "DIK_NOCONVERT";
        case DIK_YEN: return "DIK_YEN";
        case DIK_ABNT_C2: return "DIK_ABNT_C2";
        case DIK_NUMPADEQUALS: return "DIK_NUMPADEQUALS";
        case DIK_PREVTRACK: return "DIK_PREVTRACK";
        case DIK_AT: return "DIK_AT";
        case DIK_COLON: return "DIK_COLON";
        case DIK_UNDERLINE: return "DIK_UNDERLINE";
        case DIK_KANJI: return "DIK_KANJI";
        case DIK_STOP: return "DIK_STOP";
        case DIK_AX: return "DIK_AX";
        case DIK_UNLABELED: return "DIK_UNLABELED";
        case DIK_NEXTTRACK: return "DIK_NEXTTRACK";
        case DIK_NUMPADENTER: return "DIK_NUMPADENTER";
        case DIK_RCONTROL: return "DIK_RCONTROL";
        case DIK_MUTE: return "DIK_MUTE";
        case DIK_CALCULATOR: return "DIK_CALCULATOR";
        case DIK_PLAYPAUSE: return "DIK_PLAYPAUSE";
        case DIK_MEDIASTOP: return "DIK_MEDIASTOP";
        case DIK_VOLUMEDOWN: return "DIK_VOLUMEDOWN";
        case DIK_VOLUMEUP: return "DIK_VOLUMEUP";
        case DIK_WEBHOME: return "DIK_WEBHOME";
        case DIK_NUMPADCOMMA: return "DIK_NUMPADCOMMA";
        case DIK_DIVIDE: return "DIK_DIVIDE";
        case DIK_SYSRQ: return "DIK_SYSRQ";
        case DIK_RMENU: return "DIK_RMENU";
        case DIK_PAUSE: return "DIK_PAUSE";
        case DIK_HOME: return "DIK_HOME";
        case DIK_UP: return "DIK_UP";
        case DIK_PRIOR: return "DIK_PRIOR";
        case DIK_LEFT: return "DIK_LEFT";
        case DIK_RIGHT: return "DIK_RIGHT";
        case DIK_END: return "DIK_END";
        case DIK_DOWN: return "DIK_DOWN";
        case DIK_NEXT: return "DIK_NEXT";
        case DIK_INSERT: return "DIK_INSERT";
        case DIK_DELETE: return "DIK_DELETE";
        case DIK_LWIN: return "DIK_LWIN";
        case DIK_RWIN: return "DIK_RWIN";
        case DIK_APPS: return "DIK_APPS";
        case DIK_POWER: return "DIK_POWER";
        case DIK_SLEEP: return "DIK_SLEEP";
        case DIK_WAKE: return "DIK_WAKE";
        case DIK_WEBSEARCH: return "DIK_WEBSEARCH";
        case DIK_WEBFAVORITES: return "DIK_WEBFAVORITES";
        case DIK_WEBREFRESH: return "DIK_WEBREFRESH";
        case DIK_WEBSTOP: return "DIK_WEBSTOP";
        case DIK_WEBFORWARD: return "DIK_WEBFORWARD";
        case DIK_WEBBACK: return "DIK_WEBBACK";
        case DIK_MYCOMPUTER: return "DIK_MYCOMPUTER";
        case DIK_MAIL: return "DIK_MAIL";
        case DIK_MEDIASELECT: return "DIK_MEDIASELECT";
    }
    return "unknown";
}
