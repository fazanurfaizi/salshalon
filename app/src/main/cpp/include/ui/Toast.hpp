#pragma once

#include <string>

#include "graphics/Color.hpp"
#include "math/Math.hpp"

namespace salshalon::graphics {
class Texture;
}
namespace salshalon::ui {
class Renderer;
}

namespace salshalon::ui {

enum class ToastKind {
  Info,
  Warning,
  Error,
  Success,
  Custom,
};

enum class ToastAnchor {
  Center,
  BottomCenter,
  TopCenter,
};

class Toast {
public:
  struct Style {
    graphics::Color background{0.78f, 0.86f, 0.90f, 1.0f};
    graphics::Color border{0.36f, 0.62f, 0.94f, 1.0f};
    float borderThickness = 2.0f;
    float boxWidth = 600.0f;
    float padding = 24.0;

    graphics::Color titleColor{0.08f, 0.14f, 0.28f, 1.0f};
    float titleScale = 3.4;

    graphics::Color bodyColor{0.16f, 0.22f, 0.36f, 1.0f};
    float bodyScale = 2.6;

    math::Vec2 iconSize{48.0f, 48.0f};
    graphics::Color iconTint{1.0f, 1.0f, 1.0f, 1.0f};
    float iconGap = 16.0;

    float titleBodyGap = 6.0f;
  };

  Toast() = default;

  void show(std::string message, float durationSeconds = 2.5f);

  void dismiss();

  void update(float deltaSeconds);

  void render(Renderer &ui, const math::Vec2 &anchor,
              ToastAnchor anchorMode = ToastAnchor::Center) const;

  bool visible() const {
    return this->timer_ > 0.0f && !this->message_.empty();
  }
  bool animating() const { return this->timer_ > 0.0f; }

  Toast &setKind(ToastKind kind);
  Toast &setStyle(Style style);
  Toast &setTitle(std::string title);
  Toast &setIcon(const graphics::Texture *texture);
  Toast &setFadeTime(float seconds);

  const Style &style() const { return this->style_; }
  const std::string &title() const { return this->title_; }
  const std::string &message() const { return this->message_; }

private:
  float measureContentHeight() const;
  bool hasIcon() const;

  std::string title_{};
  std::string message_{};
  float timer_ = 0.0f;
  float fadeTime_ = 1.0f;

  Style style_{};
  const graphics::Texture *icon_ = nullptr;
};

} // namespace salshalon::ui
