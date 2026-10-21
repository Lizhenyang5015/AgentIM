cmake_minimum_required(VERSION 3.25)

# Restores the pinned Qt Base binary package for one supported platform. This
# file is usable both from the normal configure flow and with `cmake -P`.
cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH _agentim_project_root)
set(_agentim_qt_manifest
    "${_agentim_project_root}/third_party/manifests/qt.json")

if(NOT EXISTS "${_agentim_qt_manifest}")
    message(FATAL_ERROR "Qt dependency manifest is missing: ${_agentim_qt_manifest}")
endif()

if(NOT AGENTIM_QT_PLATFORM)
    set(_agentim_qt_host_processor "${CMAKE_HOST_SYSTEM_PROCESSOR}")
    if(APPLE AND NOT _agentim_qt_host_processor)
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE _agentim_qt_host_processor
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    elseif(WIN32 AND NOT _agentim_qt_host_processor)
        set(_agentim_qt_host_processor "$ENV{PROCESSOR_ARCHITECTURE}")
    endif()

    if(APPLE AND _agentim_qt_host_processor STREQUAL "arm64")
        set(AGENTIM_QT_PLATFORM "macos-arm64")
    elseif(WIN32 AND _agentim_qt_host_processor MATCHES "^(AMD64|amd64|x86_64)$")
        set(AGENTIM_QT_PLATFORM "windows-x64")
    else()
        message(FATAL_ERROR
            "Unable to infer a supported Qt platform. Set "
            "-DAGENTIM_QT_PLATFORM=macos-arm64 or windows-x64.")
    endif()
endif()

file(READ "${_agentim_qt_manifest}" _agentim_qt_manifest_json)
string(JSON _agentim_qt_version GET "${_agentim_qt_manifest_json}" version)
string(JSON _agentim_qt_relative_root GET
    "${_agentim_qt_manifest_json}" platforms "${AGENTIM_QT_PLATFORM}" root)
string(JSON _agentim_qt_repository GET
    "${_agentim_qt_manifest_json}" platforms "${AGENTIM_QT_PLATFORM}" acquisition repository)
string(JSON _agentim_qt_package GET
    "${_agentim_qt_manifest_json}" platforms "${AGENTIM_QT_PLATFORM}" acquisition package)
string(JSON _agentim_qt_archive GET
    "${_agentim_qt_manifest_json}" platforms "${AGENTIM_QT_PLATFORM}" acquisition archive)
string(JSON _agentim_qt_sha1 GET
    "${_agentim_qt_manifest_json}" platforms "${AGENTIM_QT_PLATFORM}" acquisition sha1)

if(NOT AGENTIM_QT_ROOT)
    set(AGENTIM_QT_ROOT "${_agentim_project_root}/${_agentim_qt_relative_root}")
endif()

if(EXISTS "${AGENTIM_QT_ROOT}/lib/cmake/Qt6/Qt6Config.cmake"
   AND EXISTS "${AGENTIM_QT_ROOT}/lib/cmake/Qt6Widgets/Qt6WidgetsConfig.cmake")
    message(STATUS "Qt SDK is already staged at ${AGENTIM_QT_ROOT}")
    return()
endif()

set(AGENTIM_QT_BASE_URL "${_agentim_qt_repository}" CACHE STRING
    "Qt online repository root or trusted mirror")
set(_agentim_qt_url
    "${AGENTIM_QT_BASE_URL}/${_agentim_qt_package}/${_agentim_qt_archive}")
set(_agentim_qt_download_root
    "${_agentim_project_root}/third_party/downloads/qt/${AGENTIM_QT_PLATFORM}/${_agentim_qt_version}")
set(_agentim_qt_archive_path
    "${_agentim_qt_download_root}/${_agentim_qt_archive}")
set(_agentim_qt_staging_root "${AGENTIM_QT_ROOT}.extracting")

file(MAKE_DIRECTORY "${_agentim_qt_download_root}")

set(_agentim_qt_download_required TRUE)
if(EXISTS "${_agentim_qt_archive_path}")
    file(SHA1 "${_agentim_qt_archive_path}" _agentim_existing_qt_sha1)
    if("${_agentim_existing_qt_sha1}" STREQUAL "${_agentim_qt_sha1}")
        set(_agentim_qt_download_required FALSE)
    else()
        file(REMOVE "${_agentim_qt_archive_path}")
    endif()
endif()

if(_agentim_qt_download_required)
    message(STATUS "Downloading Qt Base ${_agentim_qt_version} for ${AGENTIM_QT_PLATFORM}")
    file(DOWNLOAD
        "${_agentim_qt_url}"
        "${_agentim_qt_archive_path}.part"
        EXPECTED_HASH "SHA1=${_agentim_qt_sha1}"
        SHOW_PROGRESS
        STATUS _agentim_qt_download_status
        TLS_VERIFY ON
    )
    list(GET _agentim_qt_download_status 0 _agentim_qt_download_status_code)
    list(GET _agentim_qt_download_status 1 _agentim_qt_download_status_message)
    if(NOT _agentim_qt_download_status_code EQUAL 0)
        file(REMOVE "${_agentim_qt_archive_path}.part")
        message(FATAL_ERROR
            "Failed to download Qt Base: ${_agentim_qt_download_status_message}\n"
            "Set AGENTIM_QT_BASE_URL to a trusted mirror when the official "
            "repository is unavailable.")
    endif()
    file(RENAME
        "${_agentim_qt_archive_path}.part"
        "${_agentim_qt_archive_path}")
endif()

file(REMOVE_RECURSE "${_agentim_qt_staging_root}")
file(MAKE_DIRECTORY "${_agentim_qt_staging_root}")
message(STATUS "Extracting Qt SDK to ${AGENTIM_QT_ROOT}")
file(ARCHIVE_EXTRACT
    INPUT "${_agentim_qt_archive_path}"
    DESTINATION "${_agentim_qt_staging_root}"
)

if(NOT EXISTS "${_agentim_qt_staging_root}/lib/cmake/Qt6/Qt6Config.cmake"
   OR NOT EXISTS "${_agentim_qt_staging_root}/lib/cmake/Qt6Widgets/Qt6WidgetsConfig.cmake")
    message(FATAL_ERROR
        "The Qt Base archive does not contain the expected SDK layout.")
endif()

cmake_path(GET AGENTIM_QT_ROOT PARENT_PATH _agentim_qt_root_parent)
file(MAKE_DIRECTORY "${_agentim_qt_root_parent}")
file(REMOVE_RECURSE "${AGENTIM_QT_ROOT}")
file(RENAME "${_agentim_qt_staging_root}" "${AGENTIM_QT_ROOT}")
message(STATUS "Qt SDK staged successfully at ${AGENTIM_QT_ROOT}")
