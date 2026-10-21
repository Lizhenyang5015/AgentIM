target_sources(agentim_desktop PRIVATE
    src/application_info.cpp
)

target_include_directories(agentim_desktop
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
