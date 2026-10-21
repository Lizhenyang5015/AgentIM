cmake_minimum_required(VERSION 3.25)

# Downloads and stages one pinned CEF minimal binary distribution. This script
# is intentionally separate from the normal configure/build path so regular
# builds never depend on network access.
set(AGENTIM_CEF_VERSION
    "154.0.34+g14c5a08+chromium-154.0.8037.98"
    CACHE STRING "Pinned CEF binary distribution version")
set(AGENTIM_CEF_BASE_URL
    "https://cef-builds.spotifycdn.com"
    CACHE STRING "CEF binary distribution base URL or internal mirror")

if(NOT AGENTIM_CEF_PLATFORM)
    set(_agentim_cef_host_processor "${CMAKE_HOST_SYSTEM_PROCESSOR}")
    if(APPLE AND NOT _agentim_cef_host_processor)
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE _agentim_cef_host_processor
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    elseif(WIN32 AND NOT _agentim_cef_host_processor)
        set(_agentim_cef_host_processor "$ENV{PROCESSOR_ARCHITECTURE}")
    endif()

    if(APPLE AND _agentim_cef_host_processor STREQUAL "arm64")
        set(AGENTIM_CEF_PLATFORM "macos-arm64")
    elseif(WIN32 AND _agentim_cef_host_processor MATCHES "^(AMD64|amd64|x86_64)$")
        set(AGENTIM_CEF_PLATFORM "windows-x64")
    else()
        message(FATAL_ERROR
            "Unable to infer a supported CEF platform. Set "
            "-DAGENTIM_CEF_PLATFORM=macos-arm64 or windows-x64.")
    endif()
endif()

if(AGENTIM_CEF_PLATFORM STREQUAL "macos-arm64")
    set(_agentim_cef_archive_platform "macosarm64")
elseif(AGENTIM_CEF_PLATFORM STREQUAL "windows-x64")
    set(_agentim_cef_archive_platform "windows64")
else()
    message(FATAL_ERROR
        "Unsupported AGENTIM_CEF_PLATFORM '${AGENTIM_CEF_PLATFORM}'. "
        "Supported values: macos-arm64, windows-x64.")
endif()

cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH _agentim_project_root)
set(_agentim_cef_platform_root
    "${_agentim_project_root}/third_party/cef/${AGENTIM_CEF_PLATFORM}/${AGENTIM_CEF_VERSION}")
if(AGENTIM_CEF_ROOT)
    set(_agentim_cef_sdk_root "${AGENTIM_CEF_ROOT}")
else()
    set(_agentim_cef_sdk_root "${_agentim_cef_platform_root}/sdk")
endif()
set(_agentim_cef_distribution
    "cef_binary_${AGENTIM_CEF_VERSION}_${_agentim_cef_archive_platform}_minimal")
set(_agentim_cef_archive "${_agentim_cef_distribution}.tar.bz2")
set(_agentim_cef_archive_path "${_agentim_cef_platform_root}/${_agentim_cef_archive}")
set(_agentim_cef_sha1_path "${_agentim_cef_archive_path}.sha1")
set(_agentim_cef_staging_root "${_agentim_cef_platform_root}/.extracting")

if(EXISTS "${_agentim_cef_sdk_root}/cmake/FindCEF.cmake"
   AND EXISTS "${_agentim_cef_sdk_root}/include/cef_app.h")
    message(STATUS "CEF SDK is already staged at ${_agentim_cef_sdk_root}")
    return()
endif()

file(MAKE_DIRECTORY "${_agentim_cef_platform_root}")

string(REPLACE "+" "%2B" _agentim_cef_archive_url
    "${AGENTIM_CEF_BASE_URL}/${_agentim_cef_archive}")

if(NOT EXISTS "${_agentim_cef_sha1_path}")
    message(STATUS "Downloading CEF checksum: ${_agentim_cef_archive_url}.sha1")
    file(DOWNLOAD
        "${_agentim_cef_archive_url}.sha1"
        "${_agentim_cef_sha1_path}"
        STATUS _agentim_cef_sha1_status
        TLS_VERIFY ON
    )
    list(GET _agentim_cef_sha1_status 0 _agentim_cef_sha1_status_code)
    list(GET _agentim_cef_sha1_status 1 _agentim_cef_sha1_status_message)
    if(NOT _agentim_cef_sha1_status_code EQUAL 0)
        message(FATAL_ERROR
            "Failed to download the CEF checksum: ${_agentim_cef_sha1_status_message}\n"
            "If the public host is unavailable on your network, rerun with "
            "-DAGENTIM_CEF_BASE_URL=<trusted-internal-mirror>.")
    endif()
endif()

file(READ "${_agentim_cef_sha1_path}" _agentim_cef_sha1)
string(STRIP "${_agentim_cef_sha1}" _agentim_cef_sha1)
string(LENGTH "${_agentim_cef_sha1}" _agentim_cef_sha1_length)
if(NOT _agentim_cef_sha1 MATCHES "^[0-9A-Fa-f]+$"
   OR NOT _agentim_cef_sha1_length EQUAL 40)
    message(FATAL_ERROR
        "The downloaded CEF checksum is invalid: '${_agentim_cef_sha1}'.")
endif()

set(_agentim_cef_download_required TRUE)
if(EXISTS "${_agentim_cef_archive_path}")
    file(SHA1 "${_agentim_cef_archive_path}" _agentim_existing_cef_sha1)
    if("${_agentim_existing_cef_sha1}" STREQUAL "${_agentim_cef_sha1}")
        set(_agentim_cef_download_required FALSE)
    else()
        file(REMOVE "${_agentim_cef_archive_path}")
    endif()
endif()

if(_agentim_cef_download_required)
    message(STATUS "Downloading CEF minimal distribution: ${_agentim_cef_archive_url}")
    file(DOWNLOAD
        "${_agentim_cef_archive_url}"
        "${_agentim_cef_archive_path}.part"
        EXPECTED_HASH "SHA1=${_agentim_cef_sha1}"
        SHOW_PROGRESS
        STATUS _agentim_cef_download_status
        TLS_VERIFY ON
    )
    list(GET _agentim_cef_download_status 0 _agentim_cef_download_status_code)
    list(GET _agentim_cef_download_status 1 _agentim_cef_download_status_message)
    if(NOT _agentim_cef_download_status_code EQUAL 0)
        file(REMOVE "${_agentim_cef_archive_path}.part")
        message(FATAL_ERROR
            "Failed to download the CEF archive: ${_agentim_cef_download_status_message}")
    endif()
    file(RENAME "${_agentim_cef_archive_path}.part" "${_agentim_cef_archive_path}")
endif()

file(REMOVE_RECURSE "${_agentim_cef_staging_root}")
file(MAKE_DIRECTORY "${_agentim_cef_staging_root}")
message(STATUS "Extracting CEF SDK to ${_agentim_cef_sdk_root}")
file(ARCHIVE_EXTRACT
    INPUT "${_agentim_cef_archive_path}"
    DESTINATION "${_agentim_cef_staging_root}"
)

set(_agentim_extracted_cef_root
    "${_agentim_cef_staging_root}/${_agentim_cef_distribution}")
if(NOT EXISTS "${_agentim_extracted_cef_root}/cmake/FindCEF.cmake"
   OR NOT EXISTS "${_agentim_extracted_cef_root}/include/cef_app.h")
    message(FATAL_ERROR
        "The extracted archive does not contain the expected CEF SDK layout.")
endif()

file(REMOVE_RECURSE "${_agentim_cef_sdk_root}")
file(RENAME "${_agentim_extracted_cef_root}" "${_agentim_cef_sdk_root}")
file(REMOVE_RECURSE "${_agentim_cef_staging_root}")
message(STATUS "CEF SDK staged successfully at ${_agentim_cef_sdk_root}")
