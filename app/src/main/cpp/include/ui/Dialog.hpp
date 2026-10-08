#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <utility>

#include "graphics/Color.hpp"
#include "math/Math.hpp"
#include "math/Rect.hpp"

namespace salshalon::input {
struct TouchEvent;
}

namespace salshalon::ui {

class Renderer;

class Dialog {
public:
  struct Style {
    graphics::Color backdrop{0.0f, 0.0f, 0.0f, 0.66f};
    graphics::Color panel{0.14f, 0.16f, 0.23f, 1.0f};
    graphics::Color border{1.0f, 1.0f, 1.0f, 0.30f};
    graphics::Color titleColor{1.0f, 1.0f, 1.0f, 1.0f};
    graphics::Color bodyColor{0.80f, 0.86f, 0.94f, 1.0f};
    graphics::Color hintColor{0.58f, 0.63f, 0.72f, 1.0f};

    float borderThickness = 3.0f;

    float panelWidth = 580.0f;
    float panelHeight = 420.0f;

    float titleScale = 4.5f;
    float bodyScale = 3.0f;
    float hintScale = 2.2f;

    float titleTopInset = 80.0f;
    float hintBottomInset = 70.0f;
  };

  Dialog() = default;

  void setStyle(Style style) { this->style_ = std::move(style); }
  const Style &style() const { return this->style_; }

  void setTitle(std::string title) { this->title_ = std::move(title); }
  void setBody(std::string body) { this->body_ = std::move(body); }
  void setHint(std::string hint) { this->hint_ = std::move(hint); }

  void setDismissOnTap(bool value) { this->dismissOnTap_ = value; }

  void setOnDismiss(std::function<void()> callback) {
    this->onDismiss_ = std::move(callback);
  }

  void show();
  void hide();
  bool visible() const { return this->visible_; }

  /// True while the open/close animation is still producing pixels.
  bool rendering() const { return visible_ || openAnim_ > 0.01f; }

  void update(float deltaSeconds);

  /// Draws the backdrop and panel centred inside `viewport`.
  void render(Renderer &renderer, const math::Rect &viewport) const;

  /// Returns true if the event was consumed by this dialog.
  bool onTouch(const input::TouchEvent &event);

private:
  Style style_{};

  std::string title_{};
  std::string body_{};
  std::string hint_{};

  std::function<void()> onDismiss_{};

  bool visible_ = false;
  bool dismissOnTap_ = true;
  float openAnim_ = 0.0f;
};

} // namespace salshalon::ui
