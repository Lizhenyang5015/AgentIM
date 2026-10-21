cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(AgentIMWorkbench VERSION 0.1.0 LANGUAGES NONE)
endif()

set(_agentim_workbench_manifest
    "${CMAKE_CURRENT_SOURCE_DIR}/app.manifest.json")
set(_agentim_workbench_package
    "${CMAKE_CURRENT_SOURCE_DIR}/package.json")

file(READ "${_agentim_workbench_manifest}"
    _agentim_workbench_manifest_json)
file(READ "${_agentim_workbench_package}"
    _agentim_workbench_package_json)

string(JSON _agentim_workbench_entry_point GET
    "${_agentim_workbench_manifest_json}" entryPoint)
string(JSON _agentim_workbench_app_id GET
    "${_agentim_workbench_manifest_json}" appId)
string(JSON _agentim_workbench_manifest_version GET
    "${_agentim_workbench_manifest_json}" version)
string(JSON _agentim_workbench_package_version GET
    "${_agentim_workbench_package_json}" version)

if(NOT EXISTS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${_agentim_workbench_entry_point}")
    message(FATAL_ERROR
        "Workbench entry point does not exist: "
        "src/${_agentim_workbench_entry_point}")
endif()

if(NOT _agentim_workbench_manifest_version STREQUAL
        _agentim_workbench_package_version)
    message(FATAL_ERROR
        "Workbench version mismatch: app.manifest.json contains "
        "${_agentim_workbench_manifest_version}, while package.json contains "
        "${_agentim_workbench_package_version}")
endif()

file(GLOB_RECURSE _agentim_workbench_sources
    CONFIGURE_DEPENDS
    LIST_DIRECTORIES FALSE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*")

set(_agentim_workbench_dist_dir "${CMAKE_CURRENT_BINARY_DIR}/dist")
set(_agentim_workbench_build_stamp
    "${CMAKE_CURRENT_BINARY_DIR}/agentim_web_workbench.stamp")

add_custom_command(
    OUTPUT "${_agentim_workbench_build_stamp}"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
        "${_agentim_workbench_dist_dir}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${_agentim_workbench_dist_dir}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${_agentim_workbench_dist_dir}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${_agentim_workbench_manifest}"
        "${_agentim_workbench_dist_dir}/app.manifest.json"
    COMMAND "${CMAKE_COMMAND}" -E touch
        "${_agentim_workbench_build_stamp}"
    DEPENDS
        ${_agentim_workbench_sources}
        "${_agentim_workbench_manifest}"
        "${_agentim_workbench_package}"
    COMMENT
        "Building AgentIM Workbench ${_agentim_workbench_manifest_version}"
    VERBATIM
)

add_custom_target(agentim_web_workbench ALL
    DEPENDS "${_agentim_workbench_build_stamp}")

set_property(TARGET agentim_web_workbench PROPERTY
    AGENTIM_WEB_APP_DIST_DIR "${_agentim_workbench_dist_dir}")
set_property(TARGET agentim_web_workbench PROPERTY
    AGENTIM_WEB_APP_ID "${_agentim_workbench_app_id}")
set_property(TARGET agentim_web_workbench PROPERTY
    AGENTIM_WEB_APP_VERSION "${_agentim_workbench_manifest_version}")
set_property(GLOBAL APPEND PROPERTY
    AGENTIM_WEB_APP_TARGETS agentim_web_workbench)
