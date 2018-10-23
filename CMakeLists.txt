cmake_minimum_required(VERSION 3.9)
project(FishEngine-ECS)
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
ADD_SUBDIRECTORY(./ThirdParty/glfw/)
include_directories(./ThirdParty/glfw/include)
SET_TARGET_PROPERTIES(glfw PROPERTIES FOLDER "ThirdParty")

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
file(GLOB_RECURSE SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEngine/*.cpp)
add_library(FishEngine ${HEADERS} ${SRCS})


file(GLOB_RECURSE HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor/*.hpp)
file(GLOB_RECURSE SRCS
    ${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor/*.cpp)
add_library(FishEditor ${HEADERS} ${SRCS})
target_include_directories(FishEditor PUBLIC "./ThirdParty/imgui")

AutoGroup(./Include/FishEngine Math Systems Components ECS Render)
AutoGroup(./Source/FishEngine Math Systems Components Render)

target_include_directories(FishEngine PUBLIC "./ThirdParty/tinygltf")

file(GLOB IMGUI_HEADERS "./ThirdParty/imgui/*.h"
        "./ThirdParty/imgui/examples/imgui_impl_dx12.h"
        "./ThirdParty/imgui/examples/imgui_impl_win32.h"
        "./ThirdParty/imgui/examples/imgui_impl_glfw.h")
file(GLOB IMGUI_SRCS "./ThirdParty/imgui/*.cpp"
        "./ThirdParty/imgui/examples/imgui_impl_dx12.cpp"
        "./ThirdParty/imgui/examples/imgui_impl_win32.cpp"
        "./ThirdParty/imgui/examples/imgui_impl_glfw.cpp")
add_library(imgui ${IMGUI_HEADERS} ${IMGUI_SRCS})
target_include_directories(imgui PUBLIC "./ThirdParty/imgui")
SET_TARGET_PROPERTIES(imgui PROPERTIES FOLDER "ThirdParty")

target_link_libraries(FishEngine glfw)
target_link_libraries(FishEngine imgui)
target_link_libraries(FishEngine d3d12.lib)
target_link_libraries(FishEngine d3dcompiler.lib)
target_link_libraries(FishEngine dxgi.lib)
target_link_libraries(FishEditor FishEngine)

if (MSVC)
else()
    target_compile_definitions(FishEngine PRIVATE BGFX_CONFIG_RENDERER_OPENGL=410)
	set_target_properties(FishEngine PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")
	set(BGFX_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/bgfx/.build/osx64_clang/bin")
endif()

include_directories("./Include")

add_executable(demo1 "./Examples/demo1.cpp")
target_link_libraries(demo1 FishEngine)

add_executable(demo2 "./Examples/demo2.cpp")
target_link_libraries(demo2 FishEngine)

add_executable(Editor MACOSX_BUNDLE "./Examples/model_viewer.cpp")
target_link_libraries(Editor FishEditor)


add_executable(TestD3D WIN32 "./Examples/test_d3d12.cpp")
# # target_link_libraries(TestD3D DX12Lib)
target_link_libraries(TestD3D glfw)
target_link_libraries(TestD3D FishEngine)

# ADD_SUBDIRECTORY("./Examples/Tutorial2")


set(PIX_DIR "./ThirdParty\\DirectX-Graphics-Samples\\Packages\\WinPixEventRuntime.1.0.180612001")
set(MiniEngine_ROOT_DIR "./ThirdParty/DirectX-Graphics-Samples/MiniEngine")
set(MINIENGINE_CORE_ROOT_DIR ${MiniEngine_ROOT_DIR}/Core)
file(GLOB_RECURSE HEADERS ${MINIENGINE_CORE_ROOT_DIR}/*.h)
file(GLOB_RECURSE SRCS ${MINIENGINE_CORE_ROOT_DIR}/*.cpp)
set(ZLIB_DIR "${MiniEngine_ROOT_DIR}/../Packages/zlib-vc140-static-64.1.2.11/lib/native")
link_directories("${ZLIB_DIR}/libs/x64/static/Release")
link_directories("${PIX_DIR}/bin")
add_library(MiniEngine ${HEADERS} ${SRCS})
target_include_directories(MiniEngine PRIVATE "${ZLIB_DIR}/include")
target_include_directories(MiniEngine PRIVATE "${PIX_DIR}/Include/WinPixEventRuntime")
target_include_directories(MiniEngine PUBLIC ${MINIENGINE_CORE_ROOT_DIR})
target_link_libraries(MiniEngine zlibstatic.lib)
target_link_libraries(MiniEngine WinPixEventRuntime.lib)
target_compile_definitions(MiniEngine PRIVATE _UNICODE UNICODE)
set_property(TARGET MiniEngine PROPERTY CXX_STANDARD 14)


set(ModelViewer_ROOT_DIR "./Examples/ModelViewer")
file(GLOB_RECURSE SOURCES ${ModelViewer_ROOT_DIR}/*.h ${ModelViewer_ROOT_DIR}/*.cpp)
# link_directories("${MiniEngine_ROOT_DIR}/Build_VS15/x64/Debug/Output/Core")
link_directories("${MiniEngine_ROOT_DIR}/Build_VS15/x64/Debug/Output/Model")
add_executable(ModelViewer ${SOURCES})
target_include_directories(ModelViewer PUBLIC ${MiniEngine_ROOT_DIR}/Model)
target_include_directories(ModelViewer PRIVATE "${MiniEngine_ROOT_DIR}/Build_VS15/x64/Debug/Output/ModelViewer")
# target_link_libraries(ModelViewer Core.lib)
target_link_libraries(ModelViewer MiniEngine)
target_link_libraries(ModelViewer Model.lib)
target_include_directories(ModelViewer PUBLIC "./ThirdParty/imgui")
target_link_libraries(ModelViewer imgui)
target_link_libraries(ModelViewer d3dcompiler.lib)
target_link_libraries(ModelViewer glfw)
target_link_libraries(ModelViewer FishEngine)
target_link_libraries(ModelViewer FishEditor)

# add_custom_command(TARGET ModelViewer
#     PRE_BUILD
#     COMMAND "C:/Users/yuyunkang/Downloads/nuget.exe" restore ${CMAKE_CURRENT_LIST_DIR}/build/FishEngine-ECS.sln
# )


if (MSVC)
	# target_compile_definitions(imgui PRIVATE _ITERATOR_DEBUG_LEVEL=0)
 #    target_compile_definitions(demo1 PRIVATE _ITERATOR_DEBUG_LEVEL=0)
 #    target_compile_definitions(demo2 PRIVATE _ITERATOR_DEBUG_LEVEL=0)
 #    target_compile_definitions(Editor PRIVATE _ITERATOR_DEBUG_LEVEL=0)
else()
    target_compile_definitions(demo1 PRIVATE BGFX_CONFIG_RENDERER_OPENGL=410)
    target_compile_definitions(imgui PRIVATE BGFX_CONFIG_RENDERER_OPENGL=410)
    target_compile_definitions(demo2 PRIVATE BGFX_CONFIG_RENDERER_OPENGL=410)
    target_compile_definitions(Editor PRIVATE BGFX_CONFIG_RENDERER_OPENGL=410)
    set_target_properties(demo1 PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")
    set_target_properties(demo2 PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")
    set_target_properties(Editor PROPERTIES LINK_FLAGS "-framework OpenGL -framework Cocoa -framework metal -framework QuartzCore")

    SET( INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/MacOSXBundleInfo.plist.in)
    IF (NOT EXISTS ${INFO_PLIST})
        MESSAGE(FATAL_ERROR "${INFO_PLIST} not found!")
    ENDIF()
    set_target_properties(Editor PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST}" )
endif()
