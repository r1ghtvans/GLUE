cmake_minimum_required(VERSION 3.22)
project(PEPEL VERSION 1.0.0 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)

# ---- Шрифты (OFL), качаем один раз в Assets/ ----
set(FONT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Assets)
file(MAKE_DIRECTORY ${FONT_DIR})

function(fetch_if_missing url dest)
  if(NOT EXISTS ${dest})
    message(STATUS "Downloading ${url}")
    file(DOWNLOAD ${url} ${dest} STATUS st LOG log SHOW_PROGRESS)
    list(GET st 0 code)
    if(NOT code EQUAL 0)
      message(FATAL_ERROR "Download failed: ${url}\n${log}")
    endif()
  endif()
endfunction()

fetch_if_missing(https://raw.githubusercontent.com/google/fonts/main/ofl/vt323/VT323-Regular.ttf            ${FONT_DIR}/VT323-Regular.ttf)
fetch_if_missing(https://raw.githubusercontent.com/google/fonts/main/ofl/pressstart2p/PressStart2P-Regular.ttf ${FONT_DIR}/PressStart2P-Regular.ttf)
fetch_if_missing(https://raw.githubusercontent.com/google/fonts/main/ofl/vt323/OFL.txt                      ${FONT_DIR}/OFL-VT323.txt)
fetch_if_missing(https://raw.githubusercontent.com/google/fonts/main/ofl/pressstart2p/OFL.txt               ${FONT_DIR}/OFL-PressStart2P.txt)

# ---- JUCE ----
include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.4
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# AU собирается только на macOS
if(APPLE)
  set(PEPEL_FORMATS VST3 AU Standalone)
else()
  set(PEPEL_FORMATS VST3 Standalone)
endif()

juce_add_plugin(PEPEL
  COMPANY_NAME "PEPEL"
  COMPANY_WEBSITE "https://example.com"
  COMPANY_EMAIL "pepel@example.com"
  PLUGIN_MANUFACTURER_CODE Pepl
  PLUGIN_CODE Pglu
  AU_MAIN_TYPE kAudioUnitType_Effect
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  FORMATS ${PEPEL_FORMATS}
  PRODUCT_NAME "PEPEL Glue"
  VST3_CATEGORIES Fx Dynamics
  BUNDLE_ID com.pepel.glue)

juce_add_binary_data(FontsBinaryData
  NAMESPACE Fonts
  SOURCES
    ${FONT_DIR}/VT323-Regular.ttf
    ${FONT_DIR}/PressStart2P-Regular.ttf)

target_sources(PEPEL PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)

target_include_directories(PEPEL PRIVATE Source)

target_compile_definitions(PEPEL PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(PEPEL PRIVATE
  FontsBinaryData
  juce::juce_audio_utils
  juce::juce_dsp)

juce_generate_juce_header(PEPEL)

# ---- Необязательный тест DSP: cmake -B build -DPEPEL_BUILD_TESTS=ON ----
option(PEPEL_BUILD_TESTS "Build offline DSP test" OFF)
if(PEPEL_BUILD_TESTS)
  juce_add_console_app(PEPELDspTest PRODUCT_NAME "PEPELDspTest")
  target_sources(PEPELDspTest PRIVATE
    Tests/DspTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_include_directories(PEPELDspTest PRIVATE Source)
  target_compile_definitions(PEPELDspTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=1)
  target_link_libraries(PEPELDspTest PRIVATE
    FontsBinaryData juce::juce_audio_utils juce::juce_dsp)
  juce_generate_juce_header(PEPELDspTest)
endif()
