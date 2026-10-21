file(GLOB _agentim_web_application_directories
    CONFIGURE_DEPENDS
    LIST_DIRECTORIES TRUE
    "${CMAKE_CURRENT_SOURCE_DIR}/*"
)

foreach(_agentim_web_application_directory
        IN LISTS _agentim_web_application_directories)
    if(IS_DIRECTORY "${_agentim_web_application_directory}" AND
            EXISTS "${_agentim_web_application_directory}/CMakeLists.txt")
        get_filename_component(_agentim_web_application_name
            "${_agentim_web_application_directory}" NAME)
        add_subdirectory(
            "${_agentim_web_application_name}"
            "${CMAKE_CURRENT_BINARY_DIR}/${_agentim_web_application_name}"
        )
    endif()
endforeach()
