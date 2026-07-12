#pragma once
#include <string>

namespace edu::gui::notifications {

void notify(const std::string& message);
void render();
void clear();

} // namespace edu::gui::notifications
