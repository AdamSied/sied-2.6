cmake_minimum_required(VERSION 3.22)

project(SIED VERSION 2.6.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Keep the Windows plugin self-contained so friends do not need to install a
# matching Visual C++ runtime before their DAW can scan SIED.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

set(SIED_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND SIED_FORMATS AU)
endif()

juce_add_plugin(SIED
    COMPANY_NAME "Sied Audio"
    BUNDLE_ID "audio.sied.instrument"
    PLUGIN_MANUFACTURER_CODE AdSd
    PLUGIN_CODE Sied
    FORMATS ${SIED_FORMATS}
    PRODUCT_NAME "SIED"
    DESCRIPTION "A three-layer sample instrument with a curated factory library and sound-design effects"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(SIED)

juce_add_binary_data(SIEDFactoryData
    HEADER_NAME SiedFactoryData.h
    NAMESPACE SiedFactoryData
    SOURCES
        Assets/Factory/bell_vase.wav
        Assets/Factory/key_ascent.wav
        Assets/Factory/key_easy.wav
        Assets/Factory/key_island.wav
        Assets/Factory/key_multi.wav
        Assets/Factory/key_noise.wav
        Assets/Factory/key_octopus.wav
        Assets/Factory/key_topograph.wav
        Assets/Factory/lead_light.wav
        Assets/Factory/pad_avenge.wav
        Assets/Factory/pad_boost.wav
        Assets/Factory/pad_hello.wav
        Assets/Factory/pluck_glow.wav
        Assets/Factory/pluck_pig.wav
        Assets/Factory/pluck_resign.wav
        Assets/Factory/texture_vinyl.wav
        Assets/UI/DejaVuSansMono-Bold.ttf
)

target_sources(SIED PRIVATE
    Source/MacroEffects.cpp
    Source/MacroEffects.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/SiedVoice.cpp
    Source/SiedVoice.h
)

target_compile_definitions(SIED PUBLIC
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
)

target_link_libraries(SIED PRIVATE
    SIEDFactoryData
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
