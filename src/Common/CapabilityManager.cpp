#include "CapabilityManager.hpp"
#include <intercept.hpp>
#include "common.hpp"

using namespace intercept;

CapabilityManager GCapabilityManager;

CapabilityManager::CapabilityManager() {
    Signal_PreStart.connect([this]() {
        updateSQFVariable();
    });
    Signal_PreInit.connect([this]() {
        updateSQFVariable();
    });
    Signal_PostInit.connect([this]() {
        updateSQFVariable();
    });
}

void CapabilityManager::addCapability(std::string_view name) {
    capabilities.emplace_back(name);
}
bool CapabilityManager::hasCapability(std::string_view name) {
    auto found = std::find(capabilities.begin(), capabilities.end(), name);
    return found != capabilities.end();
}

void CapabilityManager::updateSQFVariable() {
    auto_array<game_value> arr(capabilities.begin(), capabilities.end());
    game_value val(arr);
    sqf::set_variable(sqf::ui_namespace(), "Intercept_CBA_capabilities"sv, val);
    sqf::set_variable(sqf::mission_namespace(), "Intercept_CBA_capabilities"sv, val);
}
