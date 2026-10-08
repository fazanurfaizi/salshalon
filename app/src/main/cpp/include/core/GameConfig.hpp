#pragma once

namespace salshalon::core {

/// Every UI coordinate in the game is expressed in this virtual resolution.
/// Camera2D letterboxes/scales it onto the real surface, so layouts never
/// need to know the physical screen size.
inline constexpr float kDesignWidth = 720.0f;
inline constexpr float kDesignHeight = 1280.0f;

} // namespace salshalon::core
