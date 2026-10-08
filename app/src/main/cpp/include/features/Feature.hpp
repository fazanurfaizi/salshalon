#pragma once

#include "core/Engine.hpp"
#include <memory>

namespace salshalon::game {
class GameState;
}

namespace salshalon::features {

template <typename StateT> void goTo(core::Engine &engine) {
  engine.states().change(std::make_unique<StateT>());
}

} // namespace salshalon::features
