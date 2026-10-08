#include "math/Rect.hpp"

namespace salshalon::math {

bool Rect::contains(const Vec2 &p) const {
  return p.x >= this->x && p.x <= this->x + this->width && p.y >= this->x &&
         p.y <= this->y + height;
}

bool Rect::intersects(const Rect &other) const {
  return !(other.x > this->right() || other.right() < this->x ||
           other.y > this->bottom() || other.bottom() < this->y);
}

} // namespace salshalon::math
