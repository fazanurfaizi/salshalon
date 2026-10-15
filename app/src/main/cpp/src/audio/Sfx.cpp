#include "audio/Sfx.hpp"

#include <aaudio/AAudio.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <mutex>

#include "core/Log.hpp"

namespace salshalon::audio {

namespace {
constexpr float kTau = 6.2831853f;
constexpr int kMaxVoices = 8;
constexpr double kStreamRetryCooldownSec = 5.0;

double monotonicSeconds() {
  timespec ts{};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1e9;
}

float sine(float phase) { return std::sin(phase); }

float square(float phase) { return (std::sin(phase) >= 0.0f ? 0.6f : -0.6f); }

float noise(uint32_t &state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return static_cast<float>(state) / static_cast<float>(UINT32_MAX) * 2.0f -
         1.0f;
}

// One envelope-shaped tone with up to 3 frequency segments.
struct Voice {
  bool active = false;
  int wave = 0; // 0 = sine, 1 = square, 2 = noise burst
  float freq[3] = {440.0f, 440.0f, 440.0f};
  float segEnd[2] = {1.0f, 1.0f}; // segment boundaries as fraction of dur
  float duration = 0.2f;
  float elapsed = 0.0f;
  float volume = 0.3f;
  float phase = 0.0f;
  double queuedAtSec = 0.0; // wall-clock enqueue time
};
} // namespace

struct AudioEngine::Impl {
  std::mutex mutex;
  AAudioStream *stream = nullptr;
  bool streamRunning = false;
  double lastOpenAttemptSec = -1e9;
  bool openFailedLogged = false;

  Voice voices[kMaxVoices]{};
  int nextVoice = 0;

  bool ambient = false;
  bool stopRequested = false;
  float ambientPhase1 = 0.0f;
  float ambientPhase2 = 0.0f;
  float ambientTime = 0.0f;
  uint32_t noiseState = 0x12345678u;

  float sampleRate() const {
    if (stream != nullptr) {
      const int32_t sr = AAudioStream_getSampleRate(stream);
      if (sr > 0)
        return static_cast<float>(sr);
    }
    return 48000.0f;
  }

  bool anyVoiceActive() const {
    for (const auto &v : voices) {
      if (v.active)
        return true;
    }
    return false;
  }
};

namespace {
aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                           void *audioData, int32_t numFrames) {
  auto *impl = static_cast<AudioEngine::Impl *>(userData);
  auto *out = static_cast<float *>(audioData);
  if (out == nullptr || numFrames <= 0)
    return AAUDIO_CALLBACK_RESULT_CONTINUE;

  const int32_t channels = AAudioStream_getChannelCount(stream);
  const int32_t ch = channels > 0 ? channels : 1;
  const float sr = impl->sampleRate();
  const float dt = 1.0f / sr;
  const double now = monotonicSeconds();

  if (!impl->mutex.try_lock()) {
    for (int32_t f = 0; f < numFrames; ++f) {
      for (int32_t c = 0; c < ch; ++c) {
        out[f * ch + c] = 0.0f;
      }
    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
  }

  bool voicesActive = false;
  for (int32_t f = 0; f < numFrames; ++f) {
    float sample = 0.0f;

    // Ambient pad
    if (impl->ambient) {
      impl->ambientTime += dt;
      impl->ambientPhase1 += kTau * 110.0f * dt;
      impl->ambientPhase2 += kTau * 164.81f * dt;
      const float tremolo =
          0.75f + 0.25f * std::sin(kTau * 0.12f * impl->ambientTime);
      sample += (sine(impl->ambientPhase1) * 0.5f +
                 sine(impl->ambientPhase2) * 0.35f) *
                0.06f * tremolo;
    }

    for (auto &v : impl->voices) {
      if (!v.active)
        continue;
      // Drop voices that waited too long for a failed stream to recover
      if (now - v.queuedAtSec > 2.0) {
        v.active = false;
        continue;
      }
      voicesActive = true;
      v.elapsed += dt;
      const float t = v.elapsed / v.duration;
      if (t >= 1.0f) {
        v.active = false;
        continue;
      }
      // Staggered chord voices sit out their delay with zero envelope
      if (v.elapsed < 0.0f)
        continue;
      const float freq = t < v.segEnd[0]
                             ? v.freq[0]
                             : (t < v.segEnd[1] ? v.freq[1] : v.freq[2]);
      v.phase += kTau * freq * dt;
      float osc = 0.0f;
      switch (v.wave) {
      case 1:
        osc = square(v.phase);
        break;
      case 2:
        osc = noise(impl->noiseState) * 0.5f;
        break;
      default:
        osc = sine(v.phase);
        break;
      }
      // Fast attack, exponential decay envelope
      const float attack = std::min(1.0f, v.elapsed / 0.012f);
      const float env = attack * std::exp(-3.0f * t);
      sample += osc * v.volume * env;
    }

    if (!voicesActive && !impl->ambient)
      impl->stopRequested = true;

    sample = std::clamp(sample * 0.6f, -1.0f, 1.0f);
    for (int32_t c = 0; c < ch; ++c) {
      out[f * ch + c] = sample;
    }
  }

  impl->mutex.unlock();
  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void errorCallback(AAudioStream *, void *, aaudio_result_t error) {
  SAL_LOGW("AudioEngine: stream error %d", static_cast<int>(error));
}
} // namespace

AudioEngine::~AudioEngine() { this->shutdown(); }

void AudioEngine::ensureImpl() {
  if (this->impl_ == nullptr)
    this->impl_ = new Impl();
}

void AudioEngine::play(Sfx sfx) {
  this->ensureImpl();

  bool needStart = false;
  const double queuedAt = monotonicSeconds();
  {
    std::lock_guard<std::mutex> lock(this->impl_->mutex);

    Voice v;
    switch (sfx) {
    case Sfx::Tap:
      v.wave = 1;
      v.freq[0] = 700.0f;
      v.freq[1] = 920.0f;
      v.freq[2] = 920.0f;
      v.segEnd[0] = 0.4f;
      v.segEnd[1] = 1.0f;
      v.duration = 0.07f;
      v.volume = 0.22f;
      break;
    case Sfx::Doorbell:
      // Classic two-tone chime
      v.wave = 0;
      v.freq[0] = 880.0f;
      v.freq[1] = 659.25f;
      v.freq[2] = 659.25f;
      v.segEnd[0] = 0.35f;
      v.segEnd[1] = 1.0f;
      v.duration = 0.38f;
      v.volume = 0.30f;
      break;
    case Sfx::Served: {
      // Bright ascending arpeggio: C5 -> E5 -> G5
      v.wave = 0;
      v.freq[0] = 523.25f;
      v.freq[1] = 659.25f;
      v.freq[2] = 783.99f;
      v.segEnd[0] = 0.33f;
      v.segEnd[1] = 0.66f;
      v.duration = 0.36f;
      v.volume = 0.30f;
      break;
    }
    case Sfx::Summary: {
      // Warm C-major triad
      const float chord[3] = {261.63f, 329.63f, 392.0f};
      for (int i = 0; i < 3; ++i) {
        Voice cv;
        cv.wave = 0;
        cv.freq[0] = cv.freq[1] = cv.freq[2] = chord[i];
        cv.segEnd[0] = cv.segEnd[1] = 1.0f;
        cv.duration = 0.8f;
        cv.volume = 0.16f;
        cv.elapsed = -static_cast<float>(i) * 0.03f;
        Voice &slot = this->impl_->voices[this->impl_->nextVoice];
        this->impl_->nextVoice = (this->impl_->nextVoice + 1) % kMaxVoices;
        slot = cv;
        slot.active = true;
        slot.queuedAtSec = queuedAt;
      }
      break;
    }
    }

    if (sfx != Sfx::Summary) {
      Voice &slot = this->impl_->voices[impl_->nextVoice];
      this->impl_->nextVoice = (impl_->nextVoice + 1) % kMaxVoices;
      slot = v;
      slot.active = true;
      slot.queuedAtSec = queuedAt;
    }

    needStart = !this->impl_->streamRunning;
  }

  if (needStart)
    this->startStream();
}

// Opens and starts the shared stream
void AudioEngine::startStream() {
  if (this->impl_ == nullptr)
    return;
  std::lock_guard<std::mutex> lock(this->impl_->mutex);
  if (this->impl_->streamRunning)
    return;

  if (this->impl_->stream == nullptr) {
    const double now = monotonicSeconds();
    if (now - this->impl_->lastOpenAttemptSec <= kStreamRetryCooldownSec)
      return;
    this->impl_->lastOpenAttemptSec = now;

    AAudioStreamBuilder *builder = nullptr;
    aaudio_result_t openResult = AAUDIO_ERROR_INTERNAL;
    AAudioStream *fresh = nullptr;
    if (AAudio_createStreamBuilder(&builder) == AAUDIO_OK) {
      AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
      AAudioStreamBuilder_setChannelCount(builder, 1);
      AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
      AAudioStreamBuilder_setPerformanceMode(
          builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
      AAudioStreamBuilder_setUsage(builder, AAUDIO_USAGE_GAME);
      AAudioStreamBuilder_setDataCallback(builder, dataCallback, this->impl_);
      AAudioStreamBuilder_setErrorCallback(builder, errorCallback, this->impl_);
      openResult = AAudioStreamBuilder_openStream(builder, &fresh);
      AAudioStreamBuilder_delete(builder);
    }
    if (openResult != AAUDIO_OK || fresh == nullptr) {
      if (!this->impl_->openFailedLogged) {
        SAL_LOGW("AudioEngine: openStream failed (%d, silent mode)",
                 static_cast<int>(openResult));
        this->impl_->openFailedLogged = true;
      }
      return;
    }
    this->impl_->stream = fresh;
  }

  if (AAudioStream_requestStart(this->impl_->stream) == AAUDIO_OK) {
    this->impl_->streamRunning = true;
    this->impl_->stopRequested = false;
  }
}

void AudioEngine::setAmbient(bool enabled) {
  this->ensureImpl();
  bool needStart = false;
  {
    std::lock_guard<std::mutex> lock(this->impl_->mutex);
    if (this->impl_->ambient == enabled)
      return;
    this->impl_->ambient = enabled;
    needStart = enabled && !this->impl_->streamRunning;
  }
  if (needStart)
    this->startStream();
}

void AudioEngine::update() {
  if (this->impl_ == nullptr)
    return;
  std::lock_guard<std::mutex> lock(this->impl_->mutex);
  if (this->impl_->stopRequested && this->impl_->streamRunning &&
      this->impl_->stream != nullptr) {
    AAudioStream_requestStop(this->impl_->stream);
    this->impl_->streamRunning = false;
    this->impl_->stopRequested = false;
  }
}

void AudioEngine::shutdown() {
  if (this->impl_ == nullptr)
    return;

  {
    std::lock_guard<std::mutex> lock(this->impl_->mutex);
    if (this->impl_->stream != nullptr) {
      if (this->impl_->streamRunning)
        AAudioStream_requestStop(this->impl_->stream);
      AAudioStream_close(this->impl_->stream);
      this->impl_->stream = nullptr;
      this->impl_->streamRunning = false;
    }
    for (auto &v : this->impl_->voices) {
      v.active = false;
    }
    this->impl_->ambient = false;
  }
  delete this->impl_;
  this->impl_ = nullptr;
}

} // namespace salshalon::audio
