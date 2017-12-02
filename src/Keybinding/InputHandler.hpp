#pragma once
#include <array>
#include <memory>
#include <vector>
#include <map>
#include <shared/containers.hpp>

struct IDirectInputDevice8A;

class Keyboard {
public:
    Keyboard(std::string name_, std::string guid_, IDirectInputDevice8A* device_);

    ///Don't call this
    void initialize();

    bool poll();
    const std::array<bool, 256>& getKeyStates();
    
private:
    std::array<bool, 256> keyState{0};
    std::string name;
    std::string guid;
    IDirectInputDevice8A* device;
    bool acquired = false;
};
class Joystick {
public:
    Joystick(std::string name_, std::string guid_, IDirectInputDevice8A* device_);

    ///Don't call this
    void initialize();

    bool poll();
    const std::array<bool, 32>& getKeyStates();
    intercept::types::r_string getName();
    int16_t getPOV();
private:
    std::array<bool, 32> keyState{0};
    int16_t povDirection;
    intercept::types::r_string name;
    intercept::types::r_string guid;
    IDirectInputDevice8A* device;
    bool acquired = false;
};


class InputHandler {
public:
    void preStart();

    std::shared_ptr<Keyboard> getKeyboard();
    std::vector<std::shared_ptr<Joystick>> getJoysticks();
    void addKeyboard(std::shared_ptr<Keyboard> move_);
    void addJoystick(std::shared_ptr<Joystick> move_);
    static std::string DIKToString(uint32_t DIKCode);


    void fireEvents();

private:
    std::shared_ptr<Keyboard> keyboard;
    std::vector<std::shared_ptr<Joystick>> joysticks;
    std::map<std::string, std::shared_ptr<Joystick>> joysticksByGUID;
    std::map<std::string, std::shared_ptr<Joystick>> joysticksByName;
};