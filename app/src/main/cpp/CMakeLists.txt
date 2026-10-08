cmake_minimum_required(VERSION 3.22.1)

project("salshalon")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -ffast-math -fno-exceptions -fno-rtti")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs/glm/CMakeLists.txt)
  add_subdirectory(libs/glm)
else()
  FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(glm)
endif()

# Include android_native_app_glue
add_library(app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

add_library(salshalon SHARED
    # Entry point
    main.cpp

    src/core/AssetManager.cpp
    src/core/Clock.cpp
    src/core/Engine.cpp
    src/core/GLContext.cpp

    src/game/StateManager.cpp

    src/graphics/Camera2D.cpp
    src/graphics/Font.cpp
    src/graphics/Shader.cpp
    src/graphics/SpriteBatch.cpp
    src/graphics/Texture.cpp

    src/input/InputManager.cpp

    src/math/Rect.cpp

    src/ui/Renderer.cpp
    src/ui/Button.cpp
    src/ui/Toast.cpp

    # Features
    src/features/main_menu/MainMenuState.cpp
    src/features/main_menu/MainMenuAssets.cpp

    src/features/salon/SalonState.cpp
    src/features/salon/SalonAssets.cpp

    src/features/settings/SettingsState.cpp
)

target_include_directories(salshalon PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

target_compile_definitions(salshalon PRIVATE
    GLM_FORCE_RADIANS
    GLM_ENABLE_EXPERIMENTAL
)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# Forces the linker to keep the JNI entry point for GameActivity
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

# Configure libraries CMake uses to link your target library.
target_link_libraries(salshalon
        # The game activity
        game-activity::game-activity_static

        glm
        EGL
        GLESv3
        jnigraphics
        android
        log
)
