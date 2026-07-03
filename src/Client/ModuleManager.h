#pragma once
#include <string>
#include <vector>
#include <functional>

namespace edu {

struct ModuleSetting {
    std::string name;
    std::vector<std::string> options;
    int* selected;
};

struct ModuleInfo {
    std::string name;
    std::string description;
    std::string category;
    bool* enabled;
    std::function<void()> toggle;
    std::vector<ModuleSetting> settings;
};

inline std::vector<ModuleInfo>& getModules() {
    static std::vector<ModuleInfo> modules;
    return modules;
}

inline void registerModule(const std::string& name, const std::string& desc,
                           const std::string& category,
                           bool* enabled, std::function<void()> toggle,
                           std::vector<ModuleSetting> settings = {}) {
    getModules().push_back({name, desc, category, enabled, std::move(toggle), std::move(settings)});
}

inline std::vector<std::string> getCategories() {
    std::vector<std::string> cats;
    for (auto& m : getModules()) {
        bool found = false;
        for (auto& c : cats) if (c == m.category) { found = true; break; }
        if (!found) cats.push_back(m.category);
    }
    return cats;
}

} // namespace edu
