cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 17)

project("Demo")

set(SOURCES
    src/${PROJECT_NAME}/main.cpp
    src/${PROJECT_NAME}/window.cpp
    src/${PROJECT_NAME}/gui.cpp

    src/${PROJECT_NAME}/external/imgui/imgui.cpp
    src/${PROJECT_NAME}/external/imgui/imgui_impl_glfw.cpp
    src/${PROJECT_NAME}/external/imgui/imgui_impl_vulkan.cpp
    src/${PROJECT_NAME}/external/imgui/imgui_draw.cpp
    src/${PROJECT_NAME}/external/imgui/imgui_widgets.cpp
)

set(INCLUDES
    src/${PROJECT_NAME}/window.h
    src/${PROJECT_NAME}/gui.h

    src/${PROJECT_NAME}/external/imgui/imgui.h
    src/${PROJECT_NAME}/external/imgui/imgui_impl_glfw.h
    src/${PROJECT_NAME}/external/imgui/imgui_impl_vulkan.h
    src/${PROJECT_NAME}/external/imgui/imgui_internal.h
    src/${PROJECT_NAME}/external/imgui-filebrowser/imfilebrowser.h
)

find_package(SVLcore REQUIRED)
find_package(SVLloader REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)


add_executable (${PROJECT_NAME}
    ${INCLUDES}
    ${SOURCES}
)


target_link_libraries(${PROJECT_NAME}
    SVLcore
	SVLloader
	Vulkan::Vulkan
    glfw
	glm::glm
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES
    $<TARGET_FILE:SVLcore> DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    $<TARGET_FILE:SVLloader> DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES
    $<TARGET_FILE:glfw> DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY
    "../resources/shaders"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/resources"
)