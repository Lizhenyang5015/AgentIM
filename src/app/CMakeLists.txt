qt_add_executable(agentim_desktop
    src/main.cpp
)

agentim_attach_web_cef(agentim_desktop)

target_compile_features(agentim_desktop PRIVATE cxx_std_20)

set_target_properties(agentim_desktop PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(WIN32)
    add_custom_command(TARGET agentim_desktop POST_BUILD
        COMMAND "${AGENTIM_WINDEPLOYQT_EXECUTABLE}"
            --qtpaths "${AGENTIM_QTPATHS_EXECUTABLE}"
            "$<IF:$<CONFIG:Debug>,--debug,--release>"
            --no-translations
            --dir "$<TARGET_FILE_DIR:agentim_desktop>"
            "$<TARGET_FILE:agentim_desktop>"
        COMMENT "Staging the Qt Windows runtime"
        VERBATIM
    )
endif()

# `cmake --install` produces the staging directory consumed by the platform
# packaging step. Build-tree runtime staging is separate from release packaging.
install(TARGETS agentim_desktop
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(MSVC)
    target_compile_options(agentim_desktop PRIVATE /W4 /permissive-)
else()
    target_compile_options(agentim_desktop PRIVATE -Wall -Wextra -Wpedantic)
endif()
