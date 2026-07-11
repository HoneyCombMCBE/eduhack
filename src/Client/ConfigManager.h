#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include "ModuleManager.h"

namespace edu {

inline void saveConfig() {
    std::ofstream file("heheboi_config.txt");
    if (!file.is_open()) return;

    for (auto& m : getModules()) {
        if (m.enabled) {
            file << m.name << "_enabled=" << (*m.enabled ? "1" : "0") << "\n";
        }
        for (auto& s : m.settings) {
            if (s.selected) {
                file << m.name << "_" << s.name << "=" << *s.selected << "\n";
            }
        }
    }
}

inline void loadConfig() {
    std::ifstream file("heheboi_config.txt");
    if (!file.is_open()) return;

    std::unordered_map<std::string, int> values;
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string valStr = line.substr(pos + 1);
        try {
            values[key] = std::stoi(valStr);
        } catch (...) {}
    }

    for (auto& m : getModules()) {
        std::string enabledKey = m.name + "_enabled";
        if (values.find(enabledKey) != values.end() && m.enabled) {
            *m.enabled = (values[enabledKey] != 0);
        }
        for (auto& s : m.settings) {
            std::string settingKey = m.name + "_" + s.name;
            if (values.find(settingKey) != values.end() && s.selected) {
                *s.selected = values[settingKey];
            }
        }
    }
}

} // namespace edu
