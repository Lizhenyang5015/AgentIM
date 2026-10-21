target_sources(agentim_desktop PRIVATE
    src/i18n/localized_strings.cpp
    src/main_window.cpp
)

target_include_directories(agentim_desktop
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Widgets brings in only the Qt Core and Qt Gui dependencies it requires.
target_link_libraries(agentim_desktop
    PRIVATE
        Qt6::Widgets
)
