cmake_minimum_required(VERSION 3.25)

# VS Code CMake Tools may invoke CMake with `-G Ninja` instead of a preset.
# Resolve the project-maintained Ninja before `project()` enables languages.
if(CMAKE_GENERATOR STREQUAL "Ninja" AND NOT CMAKE_MAKE_PROGRAM)
    if(APPLE)
        set(_agentim_ninja "${CMAKE_SOURCE_DIR}/tools/ninja/macos-arm64/ninja")
    elseif(WIN32)
        set(_agentim_ninja "${CMAKE_SOURCE_DIR}/tools/ninja/windows-x64/ninja.exe")
    endif()

    if(DEFINED _agentim_ninja AND EXISTS "${_agentim_ninja}")
        set(CMAKE_MAKE_PROGRAM "${_agentim_ninja}" CACHE FILEPATH
            "AgentIM-maintained Ninja executable" FORCE)
    endif()
endif()

project(AgentIM
    VERSION 0.1.0
    DESCRIPTION "Cross-platform intelligent instant messaging desktop client"
    LANGUAGES CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep maintained build products separate from CMake and Ninja's internal
# workspace. Presets place the workspace under out/<configuration>; build/
# contains only products that developers run or inspect directly.
if(CMAKE_BUILD_TYPE)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _agentim_build_configuration)
else()
    set(_agentim_build_configuration "debug")
endif()

set(AGENTIM_BUILD_OUTPUT_ROOT
    "${CMAKE_SOURCE_DIR}/build/${_agentim_build_configuration}"
    CACHE PATH "AgentIM local build product root")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${AGENTIM_BUILD_OUTPUT_ROOT}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${AGENTIM_BUILD_OUTPUT_ROOT}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${AGENTIM_BUILD_OUTPUT_ROOT}/lib")

# All dependencies come from AgentIM's pinned, self-maintained build output.
include(cmake/AgentIMDependencies.cmake)

# The native shell uses Qt Widgets. Qt Core and Qt Gui are its required
# transitive dependencies; QML, Quick, and WebEngine are not used. The web_cef
# module links the separately maintained CEF SDK.
# Runtime plugins (such as Cocoa) are discovered dynamically. Do not load
# CMake targets for every plugin in the project-local, minimal Qt SDK.
set(QT_SKIP_AUTO_PLUGIN_INCLUSION ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

include(GNUInstallDirs)
include(cmake/AgentIMCefRuntime.cmake)

# Web applications are independent build and release units. Their build
# targets produce deployable packages; native modules consume only those
# packages and never stage files directly from Web source directories.
add_subdirectory(web_apps)

add_subdirectory(src)
