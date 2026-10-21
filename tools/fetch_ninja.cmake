cmake_minimum_required(VERSION 3.25)

# Restores the pinned Ninja executable before project() asks CMake to enable
# the Ninja generator.
cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH _agentim_project_root)
set(AGENTIM_NINJA_VERSION "1.13.1" CACHE STRING
    "Pinned Ninja version")
set(AGENTIM_NINJA_BASE_URL
    "https://github.com/ninja-build/ninja/releases/download/v${AGENTIM_NINJA_VERSION}"
    CACHE STRING "Ninja release root or trusted mirror")

if(APPLE)
    set(_agentim_ninja_platform "macos-arm64")
    set(_agentim_ninja_archive "ninja-mac.zip")
    set(_agentim_ninja_sha256
        "da7797794153629aca5570ef7c813342d0be214ba84632af886856e8f0063dd9")
    set(_agentim_ninja_executable "ninja")
elseif(WIN32)
    set(_agentim_ninja_platform "windows-x64")
    set(_agentim_ninja_archive "ninja-win.zip")
    set(_agentim_ninja_sha256
        "26a40fa8595694dec2fad4911e62d29e10525d2133c9a4230b66397774ae25bf")
    set(_agentim_ninja_executable "ninja.exe")
else()
    message(FATAL_ERROR "AgentIM currently supports Ninja on macOS and Windows.")
endif()

if(NOT AGENTIM_NINJA_ROOT)
    set(AGENTIM_NINJA_ROOT
        "${_agentim_project_root}/tools/ninja/${_agentim_ninja_platform}")
endif()
set(_agentim_ninja_path
    "${AGENTIM_NINJA_ROOT}/${_agentim_ninja_executable}")

if(EXISTS "${_agentim_ninja_path}")
    set(CMAKE_MAKE_PROGRAM "${_agentim_ninja_path}" CACHE FILEPATH
        "AgentIM-maintained Ninja executable" FORCE)
    return()
endif()

set(_agentim_ninja_download_root
    "${_agentim_project_root}/tools/downloads/ninja/${AGENTIM_NINJA_VERSION}")
set(_agentim_ninja_archive_path
    "${_agentim_ninja_download_root}/${_agentim_ninja_archive}")
set(_agentim_ninja_staging_root "${AGENTIM_NINJA_ROOT}.extracting")
file(MAKE_DIRECTORY "${_agentim_ninja_download_root}")

set(_agentim_ninja_download_required TRUE)
if(EXISTS "${_agentim_ninja_archive_path}")
    file(SHA256 "${_agentim_ninja_archive_path}" _agentim_existing_ninja_sha256)
    if("${_agentim_existing_ninja_sha256}" STREQUAL "${_agentim_ninja_sha256}")
        set(_agentim_ninja_download_required FALSE)
    else()
        file(REMOVE "${_agentim_ninja_archive_path}")
    endif()
endif()

if(_agentim_ninja_download_required)
    message(STATUS "Downloading Ninja ${AGENTIM_NINJA_VERSION} for ${_agentim_ninja_platform}")
    file(DOWNLOAD
        "${AGENTIM_NINJA_BASE_URL}/${_agentim_ninja_archive}"
        "${_agentim_ninja_archive_path}.part"
        EXPECTED_HASH "SHA256=${_agentim_ninja_sha256}"
        SHOW_PROGRESS
        STATUS _agentim_ninja_download_status
        TLS_VERIFY ON
    )
    list(GET _agentim_ninja_download_status 0 _agentim_ninja_download_status_code)
    list(GET _agentim_ninja_download_status 1 _agentim_ninja_download_status_message)
    if(NOT _agentim_ninja_download_status_code EQUAL 0)
        file(REMOVE "${_agentim_ninja_archive_path}.part")
        message(FATAL_ERROR
            "Failed to download Ninja: ${_agentim_ninja_download_status_message}")
    endif()
    file(RENAME
        "${_agentim_ninja_archive_path}.part"
        "${_agentim_ninja_archive_path}")
endif()

file(REMOVE_RECURSE "${_agentim_ninja_staging_root}")
file(MAKE_DIRECTORY "${_agentim_ninja_staging_root}")
file(ARCHIVE_EXTRACT
    INPUT "${_agentim_ninja_archive_path}"
    DESTINATION "${_agentim_ninja_staging_root}"
)
if(NOT EXISTS "${_agentim_ninja_staging_root}/${_agentim_ninja_executable}")
    message(FATAL_ERROR "The Ninja archive does not contain the expected executable.")
endif()

file(REMOVE_RECURSE "${AGENTIM_NINJA_ROOT}")
file(RENAME "${_agentim_ninja_staging_root}" "${AGENTIM_NINJA_ROOT}")
if(NOT WIN32)
    file(CHMOD "${_agentim_ninja_path}"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()
set(CMAKE_MAKE_PROGRAM "${_agentim_ninja_path}" CACHE FILEPATH
    "AgentIM-maintained Ninja executable" FORCE)
message(STATUS "Ninja staged successfully at ${_agentim_ninja_path}")
