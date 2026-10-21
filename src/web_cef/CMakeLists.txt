if(NOT TARGET AgentIM::CEF)
    message(FATAL_ERROR
        "web_cef requires AGENTIM_ENABLE_CEF=ON and a staged CEF SDK")
endif()

if(APPLE)
    set(AGENTIM_CEF_HELPER_TARGETS)

    foreach(_helper_suffix_spec IN LISTS CEF_HELPER_APP_SUFFIXES)
        string(REPLACE ":" ";" _helper_suffix_values
            "${_helper_suffix_spec}")
        list(GET _helper_suffix_values 0 _helper_name_suffix)
        list(GET _helper_suffix_values 1 _helper_target_suffix)
        list(GET _helper_suffix_values 2 _helper_bundle_id_suffix)

        set(_helper_target "agentim_cef_helper${_helper_target_suffix}")
        set(_helper_output_name "AgentIM Helper${_helper_name_suffix}")

        add_executable("${_helper_target}" MACOSX_BUNDLE
            src/cef_helper_main_mac.cpp
        )
        target_compile_features("${_helper_target}" PRIVATE cxx_std_20)
        target_link_libraries("${_helper_target}" PRIVATE AgentIM::CEF)
        target_include_directories("${_helper_target}" PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )
        target_compile_options("${_helper_target}" PRIVATE
            -Wall -Wextra -Wpedantic
        )
        set_target_properties("${_helper_target}" PROPERTIES
            # CEF helpers contain no Qt meta-object, UI form, or Qt resource
            # sources. Do not create empty Qt autogen translation units for
            # these native subprocess targets.
            AUTOMOC OFF
            AUTOUIC OFF
            AUTORCC OFF
            OUTPUT_NAME "${_helper_output_name}"
            MACOSX_BUNDLE_BUNDLE_NAME "${_helper_output_name}"
            MACOSX_BUNDLE_GUI_IDENTIFIER
                "com.agentim.desktop.helper${_helper_bundle_id_suffix}"
            MACOSX_BUNDLE_INFO_PLIST
                "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist.in"
        )

        list(APPEND AGENTIM_CEF_HELPER_TARGETS "${_helper_target}")
    endforeach()

    set_property(GLOBAL PROPERTY
        AGENTIM_CEF_HELPER_TARGETS "${AGENTIM_CEF_HELPER_TARGETS}")

endif()

function(agentim_attach_web_cef application_target)
    get_property(_agentim_web_application_targets GLOBAL PROPERTY
        AGENTIM_WEB_APP_TARGETS)
    if(NOT _agentim_web_application_targets)
        message(FATAL_ERROR
            "web_cef requires at least one registered Web application target")
    endif()

    target_sources("${application_target}" PRIVATE
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/cef_browser_widget.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/cef_runtime.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/packaged_web_app_registry.cpp"
    )

    if(APPLE)
        set(_agentim_cef_browser_widget_mac
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/cef_browser_widget_mac.mm")
        set_source_files_properties("${_agentim_cef_browser_widget_mac}"
            PROPERTIES
                LANGUAGE CXX
                COMPILE_OPTIONS "-x;objective-c++"
        )
        target_sources("${application_target}" PRIVATE
            "${_agentim_cef_browser_widget_mac}"
        )
    endif()

    target_include_directories("${application_target}"
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include>
    )

    target_link_libraries("${application_target}"
        PRIVATE
            AgentIM::CEF
            Qt6::Widgets
    )

    if(APPLE)
        get_property(_agentim_cef_helper_targets GLOBAL PROPERTY
            AGENTIM_CEF_HELPER_TARGETS)
        agentim_stage_cef_runtime(
            "${application_target}"
            ${_agentim_cef_helper_targets}
        )
        set(_agentim_web_resource_root
            "$<TARGET_BUNDLE_DIR:${application_target}>/Contents/Resources/web")
    else()
        agentim_stage_cef_runtime("${application_target}")
        set(_agentim_web_resource_root
            "$<TARGET_FILE_DIR:${application_target}>/resources/web")
    endif()

    set(_agentim_registered_web_application_ids)
    foreach(_agentim_web_application_target
            IN LISTS _agentim_web_application_targets)
        if(NOT TARGET "${_agentim_web_application_target}")
            message(FATAL_ERROR
                "Registered Web application target does not exist: "
                "${_agentim_web_application_target}")
        endif()

        get_target_property(_agentim_web_application_dist_dir
            "${_agentim_web_application_target}" AGENTIM_WEB_APP_DIST_DIR)
        get_target_property(_agentim_web_application_id
            "${_agentim_web_application_target}" AGENTIM_WEB_APP_ID)
        if(NOT _agentim_web_application_dist_dir OR
                NOT _agentim_web_application_id)
            message(FATAL_ERROR
                "Web application target must expose AGENTIM_WEB_APP_DIST_DIR "
                "and AGENTIM_WEB_APP_ID: ${_agentim_web_application_target}")
        endif()
        if(NOT _agentim_web_application_id MATCHES "^[a-z][a-z0-9-]*$")
            message(FATAL_ERROR
                "Web application ID must be URL-safe lowercase text: "
                "${_agentim_web_application_id}")
        endif()
        if(_agentim_web_application_id IN_LIST
                _agentim_registered_web_application_ids)
            message(FATAL_ERROR
                "Duplicate Web application ID: ${_agentim_web_application_id}")
        endif()
        list(APPEND _agentim_registered_web_application_ids
            "${_agentim_web_application_id}")

        set(_agentim_web_resource_destination
            "${_agentim_web_resource_root}/${_agentim_web_application_id}")
        set(_agentim_web_stage_target
            "${_agentim_web_application_target}_stage")

        add_custom_target("${_agentim_web_stage_target}" ALL
            COMMAND "${CMAKE_COMMAND}" -E remove_directory
                "${_agentim_web_resource_destination}"
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${_agentim_web_application_dist_dir}"
                "${_agentim_web_resource_destination}"
            COMMENT
                "Staging packaged Web application ${_agentim_web_application_id}"
            VERBATIM
        )
        add_dependencies(
            "${_agentim_web_stage_target}"
            "${_agentim_web_application_target}"
        )
        add_dependencies(
            "${application_target}"
            "${_agentim_web_stage_target}"
        )
    endforeach()
endfunction()
