#pragma once

namespace salshalon::ui {
class Toast;
}

namespace salshalon::features::salon {

class Tutorial {
public:
  void begin(bool active);
  void update(float deltaSeconds, ui::Toast &toast);

  void onSeated(ui::Toast &toast);
  bool onFirstPayment(ui::Toast &toast);

  bool active() const { return active_; }
  void finish();

private:
  bool active_ = false;
  int stage_ = 0;
  float timer_ = 0.0f;
};

} // namespace salshalon::features::salon
