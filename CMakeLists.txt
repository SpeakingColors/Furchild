cmake_minimum_required(VERSION 3.22)
# The release workflow rewrites the version below from the release tag so menu, installer and release agree.
project(Furchild VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.16-security
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(FurAssets SOURCES
    assets/fonts/BarlowCondensed-Medium.ttf
    assets/fonts/BarlowCondensed-SemiBold.ttf
    assets/fonts/BarlowCondensed-Bold.ttf
    assets/fonts/JetBrainsMono-Regular.ttf
    assets/fonts/JetBrainsMono-SemiBold.ttf
    assets/fonts/Fraunces-BoldItalic.ttf)
set_target_properties(FurAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)

juce_add_plugin(Furchild
    COMPANY_NAME "Studio 87"
    PLUGIN_MANUFACTURER_CODE Stu8
    PLUGIN_CODE Furc
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Dynamics
    COPY_PLUGIN_AFTER_BUILD FALSE
    PRODUCT_NAME "Furchild")

set(FUR_SOURCES
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginProcessor.h
    src/core/Furchild.h
    src/editor/PluginEditor.cpp
    src/editor/PluginEditor.h
    src/editor/Presets.cpp
    src/editor/Presets.h
    src/editor/Dialogs.cpp
    src/editor/Dialogs.h
    src/editor/Theme.h
    src/editor/Layout.h
    src/editor/Widgets.h
    src/editor/Views.h)
target_sources(Furchild PRIVATE ${FUR_SOURCES})

target_compile_definitions(Furchild PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)

target_link_libraries(Furchild
    PRIVATE FurAssets juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# Offscreen window renderer: screenshots of the real editor and plugin self tests (not part of releases).
option(FUR_BUILD_SNAPSHOT "Build the editor snapshot tool" OFF)
if(FUR_BUILD_SNAPSHOT)
    juce_add_console_app(FurSnapshot PRODUCT_NAME "FurSnapshot")
    target_sources(FurSnapshot PRIVATE tools/snapshot.cpp ${FUR_SOURCES})
    target_compile_definitions(FurSnapshot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        JUCE_STANDALONE_APPLICATION=1 JUCE_MODAL_LOOPS_PERMITTED=1 JucePlugin_Name="Furchild" JucePlugin_VersionString="0.1.0")
    target_link_libraries(FurSnapshot PRIVATE FurAssets juce::juce_audio_utils juce::juce_recommended_config_flags)
endif()
