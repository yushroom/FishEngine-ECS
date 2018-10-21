cmake_minimum_required(VERSION 3.9)
project(FishEngine)
set(CMAKE_CXX_STANDARD 17)

# Output
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# glfw
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_USE_RETINA ON CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY(../../ThirdParty/glfw/ "./build/glfw")
# include_directories(../ThirdParty/glfw/include)
SET_TARGET_PROPERTIES(glfw PROPERTIES FOLDER "ThirdParty")


macro(AutoGroup root_dir)
	foreach(group ${ARGN})
		file(GLOB_RECURSE files ${root_dir}/${group}/*.hpp ${root_dir}/${group}/*.inl ${root_dir}/${group}/*.cpp)
		source_group(${group} FILES ${files})
	endforeach(group)
endmacro()


set(IMGUI_ROOT_DIR "../../ThirdParty/imgui")
# set(IMGUI_ROOT_DIR "./ThirdParty/bgfx/3rdparty/dear-imgui")
file(GLOB IMGUI_HEADERS "${IMGUI_ROOT_DIR}/*.h"
	"${IMGUI_ROOT_DIR}/examples/imgui_impl_metal.h"
	"${IMGUI_ROOT_DIR}/examples/imgui_impl_osx.h"
	"${IMGUI_ROOT_DIR}/examples/imgui_impl_glfw.h")
file(GLOB IMGUI_SRCS "${IMGUI_ROOT_DIR}/*.cpp"
	"${IMGUI_ROOT_DIR}/examples/imgui_impl_metal.mm"
	"${IMGUI_ROOT_DIR}/examples/imgui_impl_osx.mm"
	"${IMGUI_ROOT_DIR}/examples/imgui_impl_glfw.cpp")
add_library(imgui ${IMGUI_HEADERS} ${IMGUI_SRCS})
SET_TARGET_PROPERTIES(imgui PROPERTIES FOLDER "ThirdParty")
target_include_directories(imgui PUBLIC ${IMGUI_ROOT_DIR})
target_compile_options(imgui PRIVATE "-fobjc-arc")
# set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
# set_target_properties(imgui PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
target_include_directories(imgui PUBLIC "../../ThirdParty/glfw/include")

file(GLOB SRCS "./*.hpp" "./*.cpp" "*.mm" "*.metal")
add_executable(Editor MACOSX_BUNDLE ${SRCS})
target_include_directories(Editor PUBLIC ${IMGUI_ROOT_DIR})
target_link_libraries(Editor glfw)
target_link_libraries(Editor imgui)
set_target_properties(Editor PROPERTIES LINK_FLAGS "-framework Cocoa -framework metal -framework MetalKit -framework QuartzCore")
set_target_properties(Editor PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
target_include_directories(Editor PUBLIC "../../ThirdParty/glfw/include")

SET( INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/../../MacOSXBundleInfo.plist.in)
IF (NOT EXISTS ${INFO_PLIST})
	MESSAGE(FATAL_ERROR "${INFO_PLIST} not found!")
ENDIF()
set_target_properties(Editor PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST}" )
