#include "features/salon/play/Tutorial.hpp"

#include "ui/Toast.hpp"

namespace salshalon::features::salon {

void Tutorial::begin(bool active) {
  active_ = active;
  stage_ = 0;
  timer_ = 0.0f;
}

void Tutorial::update(float deltaSeconds, ui::Toast &toast) {
  if (!active_)
    return;
  timer_ += deltaSeconds;
  // Non-intrusive: one hint at a time, each fired by what the player does.
  if (stage_ == 0 && timer_ > 0.8f) {
    toast.setKind(ui::ToastKind::Info)
        .setTitle("Welcome!")
        .show("Tap a waiting customer to seat them");
    stage_ = 1;
  }
}

void Tutorial::onSeated(ui::Toast &toast) {
  if (!active_ || stage_ != 1)
    return;
  toast.setKind(ui::ToastKind::Info)
      .setTitle("Seated!")
      .show("Service takes a moment — watch for coins");
  stage_ = 2;
}

bool Tutorial::onFirstPayment(ui::Toast &toast) {
  if (!active_ || stage_ != 2)
    return false;
  toast.setKind(ui::ToastKind::Success)
      .setTitle("Payment collected!")
      .show("Coins earn money, happy customers earn reputation");
  stage_ = 3;
  return true;
}

void Tutorial::finish() {
  active_ = false;
  stage_ = 3;
}

} // namespace salshalon::features::salon
