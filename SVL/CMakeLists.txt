cmake_minimum_required (VERSION 3.8)

project ("SVL")

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(SOLUTION_NAME "${PROJECT_NAME}")
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
SET(COMPILE_LOADER ON CACHE BOOL "should be SVL loader compiled?")

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

SET(CMAKE_CXX_STANDARD 11)

add_compile_definitions(DLLDIR_EX)

#export
configure_file(SVL.h.in "include/SVL/SVL.h")
configure_file(definitions.h.in "include/SVL/definitions.h")

install(DIRECTORY
	"${CMAKE_CURRENT_BINARY_DIR}/include/${SOLUTION_NAME}"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


add_subdirectory ("SVLcore")
if(COMPILE_LOADER)
	add_subdirectory ("SVLloader")
endif()


