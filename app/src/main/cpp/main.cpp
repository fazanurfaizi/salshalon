#include <jni.h>

#include <game-activity/GameActivity.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include <chrono>
#include <memory>
#include <thread>

#include "core/Engine.hpp"
#include "core/Log.hpp"
#include "input/TouchEvent.hpp"

extern "C" {

/// Android motion action -> engine touch phase.
salshalon::input::TouchPhase toTouchPhase(int32_t action) {
  switch (action & AMOTION_EVENT_ACTION_MASK) {
  case AMOTION_EVENT_ACTION_DOWN:
  case AMOTION_EVENT_ACTION_POINTER_DOWN:
    return salshalon::input::TouchPhase::Down;
  case AMOTION_EVENT_ACTION_MOVE:
    return salshalon::input::TouchPhase::Move;
  case AMOTION_EVENT_ACTION_UP:
  case AMOTION_EVENT_ACTION_POINTER_UP:
    return salshalon::input::TouchPhase::Up;
  case AMOTION_EVENT_ACTION_CANCEL:
  default:
    return salshalon::input::TouchPhase::Cancel;
  }
}

/*!
 * Handles commands sent to this Android application
 * @param pApp the app the commands are coming from
 * @param cmd the command to handle
 */
void handle_cmd(android_app *app, int32_t cmd) {
  auto *engine = static_cast<salshalon::core::Engine *>(app->userData);
  if (engine == nullptr)
    return;

  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    if (app->window != nullptr)
      engine->onWindowCreated(app->window);
    break;
  case APP_CMD_TERM_WINDOW:
    engine->onWindowDestroyed();
    break;
  case APP_CMD_WINDOW_RESIZED:
  case APP_CMD_CONFIG_CHANGED:
    engine->onWindowResized();
    break;
  case APP_CMD_LOW_MEMORY:
    SAL_LOGW("System reported low memory");
    break;
  default:
    break;
  }
}

/// Drags touch input out of the glue's input buffer and into the engine.
void pumpInput(android_app *app, salshalon::core::Engine &engine) {
  android_input_buffer *buffer = android_app_swap_input_buffers(app);
  if (buffer == nullptr)
    return;

  for (uint64_t i = 0; i < buffer->motionEventsCount; ++i) {
    GameActivityMotionEvent &motion = buffer->motionEvents[i];

    const int32_t actionMasked = motion.action & AMOTION_EVENT_ACTION_MASK;
    const int32_t pointerIndex =
        (motion.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
        AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    const salshalon::input::TouchPhase phase = toTouchPhase(motion.action);

    for (int32_t p = 0; p < motion.pointerCount; ++p) {
      // POINTER_DOWN/UP only apply to the pointer that changed.
      if ((actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN ||
           actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) &&
          p != pointerIndex) {
        continue;
      }

      const GameActivityPointerAxes &pointer = motion.pointers[p];

      salshalon::input::TouchEvent event;
      event.pointerId = pointer.id;
      event.phase = phase;
      event.position = {pointer.axisValues[AMOTION_EVENT_AXIS_X],
                        pointer.axisValues[AMOTION_EVENT_AXIS_Y]}; // pixels
      event.timeMs = motion.eventTime;
      engine.handleTouchRaw(event);
    }
  }

  android_app_clear_motion_events(buffer);

  // Key events are drained by the glue's default filter; clear them so the
  // buffer does not grow unbounded.
  if (buffer->keyEventsCount > 0)
    android_app_clear_key_events(buffer);
}

/*!
 * Enable the motion events you want to handle; not handled events are
 * passed back to OS for further processing. For this example case,
 * only pointer and joystick devices are enabled.
 *
 * @param motionEvent the newly arrived GameActivityMotionEvent.
 * @return true if the event is from a pointer or joystick device,
 *         false for all other input devices.
 */
bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
  auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
  return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
          sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *app) {
  SAL_LOGI("android_main: starting");

  auto engine = std::make_unique<salshalon::core::Engine>();
  engine->init(app->activity, app->activity->assetManager);

  app->userData = engine.get();
  app->onAppCmd = handle_cmd;
  android_app_set_motion_event_filter(app, motion_event_filter_func);

  constexpr auto kFrameBudget = std::chrono::milliseconds(16); // ~60 fps
  auto nextFrameAt = std::chrono::steady_clock::now();

  while (!app->destroyRequested && engine->running()) {
    // ---- 1. Drain all pending Android events (non-blocking).
    bool drained = false;
    while (!drained) {
      int events = 0;
      android_poll_source *source = nullptr;
      const int result = ALooper_pollOnce(0, nullptr, &events,
                                          reinterpret_cast<void **>(&source));

      switch (result) {
      case ALOOPER_POLL_TIMEOUT:
      case ALOOPER_POLL_WAKE:
        drained = true;
        break;
      case ALOOPER_EVENT_ERROR:
        SAL_LOGE("ALooper_pollOnce returned an error");
        drained = true;
        break;
      case ALOOPER_POLL_CALLBACK:
        break;
      default:
        if (source != nullptr)
          source->process(app, source);
        break;
      }
    }

    // ---- 2. Feed input into the engine, run one frame.
    pumpInput(app, *engine);
    engine->frame();

    // ---- 3. Frame pacing.
    nextFrameAt += kFrameBudget;
    std::this_thread::sleep_until(nextFrameAt);
    if (std::chrono::steady_clock::now() > nextFrameAt + kFrameBudget) {
      nextFrameAt = std::chrono::steady_clock::now();
    }
  }

  engine->shutdown();
  app->userData = nullptr;
  SAL_LOGI("android_main: exiting");
}
}
