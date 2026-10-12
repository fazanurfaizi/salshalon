#pragma once

#include "core/Engine.hpp"
#include <memory>
#include <utility>

namespace salshalon::game {
class GameState;
}

namespace salshalon::features {

template <typename StateT, typename... Args>
void goTo(core::Engine &engine, Args &&...args) {
  engine.states().change(std::make_unique<StateT>(std::forward<Args>(args)...));
}

} // namespace salshalon::features
