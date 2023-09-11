cmake_minimum_required (VERSION 3.8)

project(SVLloader)

set(SOURCES
    src/${PROJECT_NAME}/${SOLUTION_NAME}/loader/loader.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/loader/gltf_loader.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/loader/img_loader.cpp
    src/${PROJECT_NAME}/${SOLUTION_NAME}/loader/ktx_loader.cpp
)

set(INCLUDES
    src/${PROJECT_NAME}/${SOLUTION_NAME}/loader/loader.h
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

target_link_libraries(${PROJECT_NAME} PRIVATE
    SVLcore
    glm::glm
    gli
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJECT_NAME}>
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