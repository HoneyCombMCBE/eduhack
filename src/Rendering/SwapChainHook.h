#pragma once

namespace edu::rendering {

bool installSwapChainHook();
void removeSwapChainHook();
bool isInstalled();
void tryLazyInit();

} // namespace edu::rendering
