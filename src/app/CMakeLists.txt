qt_add_executable(agentim_desktop
    src/main.cpp
)

agentim_attach_web_cef(agentim_desktop)

target_compile_features(agentim_desktop PRIVATE cxx_std_20)

set_target_properties(agentim_desktop PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# `cmake --install` produces the staging directory consumed by the platform
# packaging step. Qt runtime deployment is added here after Qt is built.
install(TARGETS agentim_desktop
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(MSVC)
    target_compile_options(agentim_desktop PRIVATE /W4 /permissive-)
else()
    target_compile_options(agentim_desktop PRIVATE -Wall -Wextra -Wpedantic)
endif()
