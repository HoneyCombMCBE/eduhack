#pragma once
#include <string>
#include <vector>
#include <functional>

namespace edu {

struct ModuleInfo {
    std::string name;
    std::string description;
    bool* enabled;
    std::function<void()> toggle;
};

inline std::vector<ModuleInfo>& getModules() {
    static std::vector<ModuleInfo> modules;
    return modules;
}

inline void registerModule(const std::string& name, const std::string& desc,
                           bool* enabled, std::function<void()> toggle) {
    getModules().push_back({name, desc, enabled, std::move(toggle)});
}

} // namespace edu
