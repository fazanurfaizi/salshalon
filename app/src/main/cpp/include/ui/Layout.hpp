#pragma once

#include "math/Math.hpp"
#include "math/Rect.hpp"

namespace salshalon::ui {

class VerticalLayout {
public:
  VerticalLayout(const math::Vec2 &origin, float itemWidth, float itemHeight,
                 float spacing)
      : origin_(origin), itemWidth_(itemWidth), itemHeight_(itemHeight),
        spacing_(spacing), cursorY_(origin.y) {}

  /// Next rect in stack (left-aligned at `origin.x`).
  math::Rect next() {
    const math::Rect rect{this->origin_.x, this->origin_.y, this->itemWidth_,
                          this->itemHeight_};
    this->cursorY_ += this->itemHeight_ + this->spacing_;
    return rect;
  }

  /// Next rect, horizontally centered inside a container of `containerWidth`.
  math::Rect nextCentered(float containerWidth) {
    const math::Rect rect{(containerWidth - this->itemWidth_) * 0.5f,
                          this->cursorY_, this->itemWidth_, this->itemHeight_};
    this->cursorY_ += this->itemHeight_ + this->spacing_;
    return rect;
  }

  void reset() { this->cursorY_ = this->origin_.y; }

  float totalHeight(int itemCount) const {
    if (itemCount <= 0)
      return 0.0f;
    return itemCount * this->itemHeight_ + (itemCount - 1) * this->spacing_;
  }

private:
  math::Vec2 origin_;
  float itemWidth_;
  float itemHeight_;
  float spacing_;
  float cursorY_;
};

} // namespace salshalon::ui
