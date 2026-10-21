# Stage the CEF runtime beside the application in the platform-required layout.
# Dependency discovery remains in AgentIMDependencies.cmake; this module owns
# only build-tree runtime assembly.
function(agentim_stage_cef_runtime application_target)
    if(NOT TARGET "${application_target}")
        message(FATAL_ERROR "Unknown application target: ${application_target}")
    endif()

    if(APPLE)
        if(ARGC LESS 2)
            message(FATAL_ERROR
                "agentim_stage_cef_runtime requires macOS CEF helper targets")
        endif()

        set(_application_bundle
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${application_target}.app")

        COPY_MAC_FRAMEWORK(
            "${application_target}"
            "${CEF_ROOT}/Release"
            "${_application_bundle}"
        )

        foreach(_helper_target IN LISTS ARGN)
            if(NOT TARGET "${_helper_target}")
                message(FATAL_ERROR
                    "Unknown macOS CEF helper target: ${_helper_target}")
            endif()

            get_target_property(_helper_output_name
                "${_helper_target}" OUTPUT_NAME)
            set(_helper_bundle
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_helper_output_name}.app")
            set(_embedded_helper
                "${_application_bundle}/Contents/Frameworks/${_helper_output_name}.app")

            add_dependencies("${application_target}" "${_helper_target}")
            add_custom_command(TARGET "${application_target}" POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E remove_directory
                    "${_embedded_helper}"
                COMMAND "${CMAKE_COMMAND}" -E copy_directory
                    "${_helper_bundle}"
                    "${_embedded_helper}"
                COMMENT "Embedding ${_helper_output_name}"
                VERBATIM
            )
        endforeach()
    elseif(WIN32)
        add_custom_command(TARGET "${application_target}" POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CEF_ROOT}/Release"
                "$<TARGET_FILE_DIR:${application_target}>"
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CEF_ROOT}/Resources"
                "$<TARGET_FILE_DIR:${application_target}>"
            COMMENT "Staging the CEF Windows runtime"
            VERBATIM
        )
    endif()
endfunction()
