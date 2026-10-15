#pragma once

namespace salshalon::audio {

/// Procedural sound effects + ambient music, synthesized with AAudio.
enum class Sfx {
  Tap,      // UI button press
  Doorbell, // customer spawned at the door
  Served,   // service completed / payment collected
  Summary,  // day-summary fanfare
};

class AudioEngine {
public:
  // Opaque implementation handle.
  struct Impl;

  AudioEngine() = default;
  ~AudioEngine();

  AudioEngine(const AudioEngine &) = delete;
  AudioEngine &operator=(const AudioEngine &) = delete;

  void play(Sfx sfx);

  void setAmbient(bool enabled);

  void update();

  void shutdown();

private:
  Impl *impl_ = nullptr;

  void ensureImpl();
  void startStream();
};

} // namespace salshalon::audio
