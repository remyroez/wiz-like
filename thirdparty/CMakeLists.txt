target_sources(${PROJECT_NAME} PRIVATE
    imgui/backends/imgui_impl_sdl.cpp
    imgui/backends/imgui_impl_sdlrenderer.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    imgui/backends
)
