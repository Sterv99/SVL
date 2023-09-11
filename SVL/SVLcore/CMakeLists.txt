cmake_minimum_required (VERSION 3.8)

project(SVLcore)

set(SOURCES
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/camera.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/layer.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/material.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/model.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/renderer.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/texture.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/tools.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/vertex.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/window.cpp

    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/command.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/image.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/render_pass.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/pipeline.cpp
)

set(INCLUDES
    src/${PROJECT_NAME}/${SOLUTION_NAME}/common/ErrorHandler.h

    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/camera.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/layer.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/light.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/material.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/mesh.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/model.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/renderer.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/texture.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/tools.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/vertex.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/window.h

    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/command.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/image.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/render_pass.h
    src/${PROJECT_NAME}/${SOLUTION_NAME}/graphics/pipeline.h
)

# Packages
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(gli REQUIRED)
##

# Target
add_library(${PROJECT_NAME} SHARED
    ${INCLUDES}
    ${SOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  IMPORTED_LOCATION ${CMAKE_INSTALL_BINDIR}
  IMPORTED_IMPLIB ${CMAKE_INSTALL_LIBDIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Vulkan::Vulkan
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glm::glm
    gli
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJECT_NAME}>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
)
##

# Install
install(DIRECTORY
    src/${PROJECT_NAME}/${SOLUTION_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
##

# Export
install(EXPORT ${PROJECT_NAME}Targets
   FILE ${PROJECT_NAME}Config.cmake
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
##