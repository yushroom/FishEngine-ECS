cmake_minimum_required(VERSION 3.9)
project(FishEngine-ECS)
set(CMAKE_CXX_STANDARD 17)

# Output
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(USE_GLFW "Uset glfw" OFF)

if (USE_GLFW)
    # glfw
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_USE_RETINA ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY(./ThirdParty/glfw/)
    include_directories(./ThirdParty/glfw/include)
    SET_TARGET_PROPERTIES(glfw PROPERTIES FOLDER "ThirdParty")
endif()


macro(AutoGroup root_dir)
	foreach(group ${ARGN})
		file(GLOB_RECURSE files ${root_dir}/${group}/*.hpp ${root_dir}/${group}/*.inl ${root_dir}/${group}/*.cpp)
		source_group(${group} FILES ${files})
	endforeach(group)
endmacro()


file(GLOB_RECURSE HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEngine.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEngine/*.hpp 
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEngine/*.inl)
if (MSVC)
file(GLOB_RECURSE SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEngine/*.cpp)
else()
file(GLOB_RECURSE SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEngine/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEngine/*.mm)
endif()
add_library(FishEngine ${HEADERS} ${SRCS})
if (USE_GLFW)
    target_compile_definitions(FishEngine PRIVATE USE_GLFW=1)
else()
    target_compile_definitions(FishEngine PRIVATE USE_GLFW=0)
endif()


file(GLOB_RECURSE HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor/*.hpp)
if (MSVC)
file(GLOB_RECURSE SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor/*.cpp)
else()
file(GLOB_RECURSE SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor/*.mm)
endif()
add_library(FishEditor ${HEADERS} ${SRCS})


AutoGroup(./Include/FishEngine Math Systems Components ECS Render)
AutoGroup(./Source/FishEngine Math Systems Components)

target_include_directories(FishEngine PUBLIC "./ThirdParty/tinygltf")


set(IMGUI_ROOT_DIR "./ThirdParty/imgui")
# set(IMGUI_ROOT_DIR "./ThirdParty/bgfx/3rdparty/dear-imgui")
file(GLOB IMGUI_HEADERS "${IMGUI_ROOT_DIR}/*.h")
file(GLOB IMGUI_SRCS "${IMGUI_ROOT_DIR}/*.cpp")
if (MSVC)
    set(IMGUI_PLATFORM_FILES
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_dx12.h"
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_dx12.cpp"
    )
else()
    set(IMGUI_PLATFORM_FILES
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_osx.h"
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_osx.mm"
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_metal.h"
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_metal.mm"
    )
endif()
if (USE_GLFW)
    set(IMGUI_PLATFORM_FILES ${IMGUI_PLATFORM_FILES} 
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_glfw.h"
        "${IMGUI_ROOT_DIR}/examples/imgui_impl_glfw.cpp")
endif()
add_library(imgui ${IMGUI_HEADERS} ${IMGUI_SRCS} ${IMGUI_PLATFORM_FILES})
SET_TARGET_PROPERTIES(imgui PROPERTIES FOLDER "ThirdParty")
target_include_directories(imgui PUBLIC ${IMGUI_ROOT_DIR})
# target_compile_options(imgui PRIVATE "-fobjc-arc")
# set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
set_target_properties(imgui PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
target_include_directories(imgui PUBLIC ${IMGUI_ROOT_DIR})
target_include_directories(imgui PUBLIC ${IMGUI_ROOT_DIR}/examples)

if (USE_GLFW)
    target_link_libraries(FishEngine glfw)
endif()
target_link_libraries(FishEngine imgui)
target_link_libraries(FishEditor FishEngine)

if (MSVC)
	set(BGFX_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/bgfx/.build/win64_vs2017/bin")
else()
    set_target_properties(FishEngine PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
	set_target_properties(FishEngine PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")
endif()


target_include_directories(FishEngine PUBLIC "./Include")

if (MSVC)
else()
    file(GLOB METAL_SHADERS "./Shaders/hlsl/runtime/*.metal")
    set_source_files_properties(${METAL_SHADERS} PROPERTIES LANGUAGE METAL)
endif()
add_executable(Editor MACOSX_BUNDLE "./Examples/Editor.cpp" ${METAL_SHADERS})
target_link_libraries(Editor FishEditor)

file(GLOB Editor2_FILES "${CMAKE_CURRENT_LIST_DIR}/Examples/Editor/*.*")
add_executable(Editor2 MACOSX_BUNDLE ${Editor2_FILES} ${METAL_SHADERS} "${CMAKE_CURRENT_LIST_DIR}/Examples/Editor/Base.lproj/Main.storyboard")
target_link_libraries(Editor2 FishEditor)
set_target_properties(Editor2 PROPERTIES RESOURCE "${CMAKE_CURRENT_LIST_DIR}/Examples/Editor/Base.lproj/Main.storyboard")



if (MSVC)
else()
    set_target_properties(Editor PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")

    SET( INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/MacOSXBundleInfo.plist.in)
    IF (NOT EXISTS ${INFO_PLIST})
        MESSAGE(FATAL_ERROR "${INFO_PLIST} not found!")
    ENDIF()
    set_target_properties(Editor PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST}" )
    # set_target_properties(Editor2 PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST}" )


    set_target_properties(Editor2 PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")
    set_target_properties(Editor2 PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/Examples/Editor/Info.plist" )

endif()
