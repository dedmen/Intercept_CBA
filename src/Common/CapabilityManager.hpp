#pragma once
#include <string_view>
#include <vector>


class CapabilityManager {
public:
    CapabilityManager();
    void addCapability(std::string_view name);
    bool hasCapability(std::string_view name);
    void updateSQFVariable();
private:
    std::vector<std::string> capabilities;
};

extern CapabilityManager GCapabilityManager;

#define REGISTER_CAPABILITY(x) GCapabilityManager.addCapability(#x)

