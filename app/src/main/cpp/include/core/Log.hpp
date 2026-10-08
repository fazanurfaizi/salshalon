#pragma once

#include <android/log.h>

#define SAL_LOG_TAG "Salshalon"

#define SAL_LOGD(...)                                                          \
  __android_log_print(ANDROID_LOG_DEBUG, SAL_LOG_TAG, __VA_ARGS__)
#define SAL_LOGI(...)                                                          \
  __android_log_print(ANDROID_LOG_INFO, SAL_LOG_TAG, __VA_ARGS__)
#define SAL_LOGW(...)                                                          \
  __android_log_print(ANDROID_LOG_WARN, SAL_LOG_TAG, __VA_ARGS__)
#define SAL_LOGE(...)                                                          \
  __android_log_print(ANDROID_LOG_ERROR, SAL_LOG_TAG, __VA_ARGS__)
