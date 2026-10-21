# Dependencies are AgentIM-maintained, platform-specific binary SDKs.
# Application modules consume CMake targets and never use these paths directly.
if(APPLE)
    set(_agentim_default_qt_root
        "${CMAKE_SOURCE_DIR}/third_party/qt/macos-arm64/6.10.3/macos")
    set(_agentim_default_cef_root
        "${CMAKE_SOURCE_DIR}/third_party/cef/macos-arm64/154.0.34+g14c5a08+chromium-154.0.8037.98/sdk")
elseif(WIN32)
    set(_agentim_default_qt_root
        "${CMAKE_SOURCE_DIR}/third_party/qt/windows-x64/6.10.3/msvc2022_64")
    set(_agentim_default_cef_root
        "${CMAKE_SOURCE_DIR}/third_party/cef/windows-x64/154.0.34+g14c5a08+chromium-154.0.8037.98/sdk")
else()
    message(FATAL_ERROR "AgentIM currently supports only macOS and Windows.")
endif()

set(AGENTIM_QT_ROOT "${_agentim_default_qt_root}" CACHE PATH
    "AgentIM-maintained Qt SDK root")
option(AGENTIM_AUTO_FETCH_DEPENDENCIES
    "Restore missing pinned binary SDKs during CMake configure" ON)

if(AGENTIM_AUTO_FETCH_DEPENDENCIES
   AND (NOT EXISTS "${AGENTIM_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake"
        OR NOT EXISTS "${AGENTIM_QT_ROOT}/lib/cmake/Qt6Widgets/Qt6WidgetsConfig.cmake"))
    if(APPLE)
        set(AGENTIM_QT_PLATFORM "macos-arm64")
    elseif(WIN32)
        set(AGENTIM_QT_PLATFORM "windows-x64")
    endif()
    include("${CMAKE_SOURCE_DIR}/tools/fetch_qt.cmake")
endif()

if(NOT EXISTS "${AGENTIM_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake"
   OR NOT EXISTS "${AGENTIM_QT_ROOT}/lib/cmake/Qt6Widgets/Qt6WidgetsConfig.cmake")
    message(FATAL_ERROR
        "The Qt SDK is missing or incomplete at ${AGENTIM_QT_ROOT}. "
        "Enable AGENTIM_AUTO_FETCH_DEPENDENCIES or provide AGENTIM_QT_ROOT.")
endif()

list(PREPEND CMAKE_PREFIX_PATH "${AGENTIM_QT_ROOT}")

set(AGENTIM_CEF_ROOT "${_agentim_default_cef_root}" CACHE PATH
    "AgentIM-maintained CEF minimal binary SDK root")
option(AGENTIM_ENABLE_CEF
    "Load the staged CEF SDK and build its official C++ wrapper" OFF)

if(AGENTIM_ENABLE_CEF)
    if(AGENTIM_AUTO_FETCH_DEPENDENCIES
       AND (NOT EXISTS "${AGENTIM_CEF_ROOT}/cmake/FindCEF.cmake"
            OR NOT EXISTS "${AGENTIM_CEF_ROOT}/include/cef_app.h"))
        if(APPLE)
            set(AGENTIM_CEF_PLATFORM "macos-arm64")
        elseif(WIN32)
            set(AGENTIM_CEF_PLATFORM "windows-x64")
        endif()
        include("${CMAKE_SOURCE_DIR}/tools/fetch_cef.cmake")
    endif()

    if(NOT EXISTS "${AGENTIM_CEF_ROOT}/cmake/FindCEF.cmake"
       OR NOT EXISTS "${AGENTIM_CEF_ROOT}/include/cef_app.h")
        message(FATAL_ERROR
            "The CEF SDK is missing or incomplete at ${AGENTIM_CEF_ROOT}. "
            "Enable AGENTIM_AUTO_FETCH_DEPENDENCIES or provide AGENTIM_CEF_ROOT.")
    endif()

    # The official CEF package ships its own FindCEF module and an unmodified
    # C++ wrapper that must be compiled for the consuming toolchain.
    set(CEF_ROOT "${AGENTIM_CEF_ROOT}" CACHE PATH "CEF SDK root" FORCE)
    list(PREPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")

    if(APPLE)
        set(PROJECT_ARCH "arm64")
    endif()

    find_package(CEF REQUIRED)

    if(NOT TARGET libcef_dll_wrapper)
        add_subdirectory(
            "${CEF_LIBCEF_DLL_WRAPPER_PATH}"
            "${CMAKE_BINARY_DIR}/third_party/cef/libcef_dll_wrapper"
        )
    endif()

    add_library(agentim_cef_sdk INTERFACE)
    add_library(AgentIM::CEF ALIAS agentim_cef_sdk)
    target_include_directories(agentim_cef_sdk SYSTEM INTERFACE "${CEF_ROOT}")
    target_link_libraries(agentim_cef_sdk INTERFACE
        libcef_dll_wrapper
        ${CEF_STANDARD_LIBS}
    )

    # macOS loads the bundled framework dynamically as required by the CEF
    # sandbox. Windows links the import library and loads libcef.dll at runtime.
    if(NOT APPLE)
        target_link_libraries(agentim_cef_sdk INTERFACE libcef_lib)
    endif()
endif()
