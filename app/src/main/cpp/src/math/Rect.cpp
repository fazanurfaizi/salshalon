#include "math/Rect.hpp"

namespace salshalon::math {

bool Rect::contains(const Vec2 &p) const {
  const bool hitX = p.x >= x && p.x <= x + width;
  const bool hitY = p.y >= y && p.y <= y + height;
  const bool hit = hitX && hitY;

  return p.x >= this->x && p.x <= this->x + this->width && p.y >= this->y &&
         p.y <= this->y + height;
}

bool Rect::intersects(const Rect &other) const {
  return !(other.x > this->right() || other.right() < this->x ||
           other.y > this->bottom() || other.bottom() < this->y);
}

} // namespace salshalon::math
