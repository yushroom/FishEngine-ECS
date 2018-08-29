cmake_minimum_required(VERSION 3.9)
project(FishEngine-ECS)
set(CMAKE_CXX_STANDARD 17)

# Output
set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#add_subdirectory("/Users/yushroom/program/github/ozz-animation" "ozz-animation")


file(GLOB SRCS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

add_executable(demo ${SRCS} ${HEADERS}
    # /Users/yushroom/program/github/bgfx/examples/common/bgfx_utils.cpp
    # /Users/yushroom/program/github/bgfx/examples/common/entry/entry.cpp
)

target_compile_definitions(demo PRIVATE _ITERATOR_DEBUG_LEVEL=0)

# include_directories("/usr/local/Cellar/glfw/3.2.1/include")
include_directories("D:/program/bgfx/include")
include_directories("D:/program/bimg/include")
include_directories("D:/program/bx/include")

include_directories("D:/program/bx/include/compat/msvc")
include_directories("D:/program/bgfx/3rdparty")

#target_link_libraries(main ozz_animation)
#set_target_properties(main PROPERTIES LINK_FLAGS "-lglfw -framework OpenGL -L/usr/local/Cellar/glfw/3.2.1/lib -L/Users/yushroom/program/github/bgfx/.build/osx64_clang/bin -lbgfxDebug -lbxDebug -lbimgDebug -lbimg_decodeDebug -framework Cocoa -framework metal -framework QuartzCore")

#link_directories("D:/program/bgfx/.build/win64_vs2017/bin")

# glfw
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY(./ThirdParty/glfw/)
include_directories(./ThirdParty/glfw/include)
SET_TARGET_PROPERTIES(glfw PROPERTIES FOLDER "ThirdParty")

target_link_libraries(demo D:/program/bgfx/.build/win64_vs2017/bin/bgfxDebug.lib)
target_link_libraries(demo D:/program/bgfx/.build/win64_vs2017/bin/bxDebug.lib)
target_link_libraries(demo D:/program/bgfx/.build/win64_vs2017/bin/bimgDebug.lib)
target_link_libraries(demo glfw)

# # glew
# if (MSVC)
#     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251" )
#     add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
#     include_directories(${CMAKE_CURRENT_LIST_DIR}/ThirdParty/glew/include)
#     #set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
#     ADD_SUBDIRECTORY(${CMAKE_CURRENT_LIST_DIR}/ThirdParty/glew/build/cmake)

#     # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
#     # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
#     # set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
#     # set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
#     SET_TARGET_PROPERTIES(glew PROPERTIES FOLDER "ThirdParty/glew")
#     SET_TARGET_PROPERTIES(glew_s PROPERTIES FOLDER "ThirdParty/glew")
# endif()

# # bgfx
# SET( BGFX_DIR "/Users/yushroom/program/github/bgfx")
# include_directories( ${BGFX_DIR}/include)
# SET( BGFX_LIB_DIR ${BGFX_DIR}/.build/osx64_clang/bin)
# # SET( BGFX_LIB_DEBUG ${BGFX_LIB_DIR}/libbgfxDebug.a)
# set(BGFX_LIB "")
# foreach(x bgfx bx bimg)
#     set(lib ${BGFX_LIB_DIR}/lib${x}Release.a)
#     if (NOT EXISTS ${lib})
#         message(FATAL_ERROR "bgfx lib ${lib} not found")
#     endif()
#     list(APPEND BGFX_LIB ${lib})
# endforeach(x)
# # SET( BGFX_LIB_RELEASE ${BGFX_LIB_DIR}/libbgfxRelease.a ${BGFX_LIB_DIR}/libbxRelease.a)
# # SET( BGFX_LIB ${BGFX_LIB_RELEASE})
# # if (NOT EXISTS ${BGFX_LIB_RELEASE})
# #     message(FATAL_ERROR "bgfx lib not found")
# # endif()


# # Autodesk FBX sdk
# # SET( FBXSDK_DIR "" CACHE PATH "Autodesk FBX SDK root directory" )
# if (WIN32)
#     SET( FBXSDK_DIR "D:\\library\\fbxsdk\\2018.1.1")
# else()
#     SET( FBXSDK_DIR "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/FBX SDK/2019.0")
# endif()
# # SET( FBXSDK_DIR "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/FBX SDK/2018.1.1")
# include_directories( ${FBXSDK_DIR}/include )
# IF (WIN32)
#     SET( FBXSDK_LIB_DIR ${FBXSDK_DIR}/lib/vs2015/x64 )
#     SET( FBXSDK_LIB_DEBUG ${FBXSDK_LIB_DIR}/debug/libfbxsdk-md.lib )
#     SET( FBXSDK_LIB_RELEASE ${FBXSDK_LIB_DIR}/release/libfbxsdk-md.lib )
#     SET( FBXSDK_LIB debug ${FBXSDK_LIB_DEBUG} optimized ${FBXSDK_LIB_RELEASE} )
# ELSE()
#     SET( FBXSDK_LIB_DIR ${FBXSDK_DIR}/lib/clang )
#     # SET( FBXSDK_LIB_DEBUG ${FBXSDK_LIB_DIR}/debug/libfbxsdk.a )
#     SET( FBXSDK_LIB_RELEASE ${FBXSDK_LIB_DIR}/release/libfbxsdk.a )
#     SET( FBXSDK_LIB ${FBXSDK_LIB_RELEASE})
# ENDIF()


# # nVidia PhysX
# if (MSVC)
#     SET( PhysX_ROOT_DIR "D:\\library\\PhysX-3.4-master\\PhysX_3.4" )
# else()
#     SET( PhysX_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/Physx-3.4/PhysX_3.4 )
# endif()
# include_directories( ${PhysX_ROOT_DIR}/Include )
# include_directories( ${PhysX_ROOT_DIR}/../PxShared/include )
# include_directories( ${PhysX_ROOT_DIR}/Snippets )
# if (NOT EXISTS ${PhysX_ROOT_DIR}/Include/PxPhysicsAPI.h)
#     message(FATAL_ERROR "Physx not found")
# endif()
# # if (MSVC)
# #     FILE(GLOB PhysXSDK_LIBRARIES_DEBUG ${PhysX_ROOT_DIR}/Lib/vc15win64/*DEBUG*lib ${PhysX_ROOT_DIR}/../PxShared/lib/vc15win64/*DEBUG*lib)
# #     FILE(GLOB PhysXSDK_LIBRARIES_CHECKED ${PhysX_ROOT_DIR}/Lib/vc15win64/*CHECKED*lib ${PhysX_ROOT_DIR}/../PxShared/lib/vc15win64/*CHECKED*lib)
# #     SET(PhysXSDK_LIBRARIES optimized ${PhysXSDK_LIBRARIES_CHECKED} debug ${PhysXSDK_LIBRARIES_DEBUG})
# # else()
# #     FILE(GLOB PhysXSDK_LIBRARIES ${PhysX_ROOT_DIR}/Lib/osx64/*.a ${PhysX_ROOT_DIR}/../PxShared/lib/osx64/*.a)
# # endif()
# if (MSVC)
#     # PhysXSDK
#     # set(PhysX_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/PhysXSDK)
#     set(PhysXSDK_LIBRARIES_DIR "D:\\program\\FishEngine-Experiment\\ThirdParty\\PhysX-3.4\\PhysX_3.4\\Lib\\vc15win64")
#     set(PhysXSDK_LIBRARIES "")
#     foreach(x PhysX3 PhysX3Common PxPvdSDK PxFoundation)
#         set(lib1 ${PhysXSDK_LIBRARIES_DIR}/${x}DEBUG_x64.lib)
#         set(lib2 ${PhysXSDK_LIBRARIES_DIR}/${x}CHECKED_x64.lib)
#         IF(NOT EXISTS ${lib1})
#             message(FATAL_ERROR "physx root directory does not exist: ${lib1}")
#         endif ()
#         IF(NOT EXISTS ${lib2})
#             message(FATAL_ERROR "physx root directory does not exist: ${lib2}")
#         endif ()
#         list (APPEND PhysXSDK_LIBRARIES debug ${lib1} optimized ${lib2})
#     endforeach()
#     foreach(x PhysX3Extensions)
#         set(lib1 ${PhysXSDK_LIBRARIES_DIR}/${x}DEBUG.lib)
#         set(lib2 ${PhysXSDK_LIBRARIES_DIR}/${x}CHECKED.lib)
#         IF(NOT EXISTS ${lib1})
#             message(FATAL_ERROR "physx root directory does not exist: ${lib1}")
#         endif ()
#         IF(NOT EXISTS ${lib2})
#             message(FATAL_ERROR "physx root directory does not exist: ${lib2}")
#         endif ()
#         list (APPEND PhysXSDK_LIBRARIES debug ${lib1} optimized ${lib2})
#     endforeach()
# else()
#     # FILE(GLOB PhysXSDK_LIBRARIES ./ThirdParty/Physx-3.4/Lib/osx64/*.a)
#     FILE(GLOB PhysXSDK_LIBRARIES ${PhysX_ROOT_DIR}/Lib/osx64/*.a ${PhysX_ROOT_DIR}/../PxShared/lib/osx64/*.a)
# endif()
# #message("PhysXSDK_LIBRARIES" ${PhysXSDK_LIBRARIES})


# set(NANOVG_DIR "${CMAKE_CURRENT_LIST_DIR}/ThirdParty/nanovg/nanovg")
# include_directories(${NANOVG_DIR}/../)
# file(GLOB_RECURSE NANOVG_HEADERS ${NANOVG_DIR}/*.h)
# add_library(nanovg ${NANOVG_DIR}/nanovg.c ${NANOVG_HEADERS})
# set_target_properties(nanovg PROPERTIES FOLDER "ThirdParty")


# include_directories(./ThirdParty/yaml-cpp/include)
# set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
# set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# #set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
# ADD_SUBDIRECTORY(./ThirdParty/yaml-cpp)
# SET_TARGET_PROPERTIES(yaml-cpp PROPERTIES FOLDER "ThirdParty/yaml-cpp")
# SET_TARGET_PROPERTIES(format PROPERTIES FOLDER "ThirdParty/yaml-cpp")


# macro(AutoGroup root_dir)
# 	foreach(group ${ARGN})
# 		file(GLOB_RECURSE files ${root_dir}/${group}/*.hpp ${root_dir}/${group}/*.inl ${root_dir}/${group}/*.cpp)
# 		source_group(${group} FILES ${files})
# 	endforeach(group)
# endmacro()

# include_directories(${CMAKE_CURRENT_LIST_DIR}/Include/)


# # python3
# find_package(PythonInterp 3 REQUIRED)
# find_package(PythonLibs 3 REQUIRED)
# INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIR})


# if (APPLE)
# 	find_package(Boost REQUIRED COMPONENTS filesystem)
# 	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# 	find_package(folly REQUIRED)
# 	include_directories(${FOLLY_INCLUDE_DIR})
# endif()

# add_subdirectory(./ThirdParty/pybind11)
# include_directories(./ThirdParty/pybind11/include)


# # glm
# include_directories(./ThirdParty/glm)
# #FishGUI
# SET(FishGUI_DIR ../FishGUI)
# SET(FishGUI_INCLUDE_DIR ${FishGUI_DIR}/include)
# SET(FishGUI_SRC_DIR ${FishGUI_DIR}/src)
# # aux_source_directory(${FishGUI_SRC_DIR} SRCS)
# file(GLOB_RECURSE SRCS ${FishGUI_SRC_DIR}/*.cpp)
# FILE(GLOB_RECURSE HEADERS ${FishGUI_DIR}/include/FishGUI/*.hpp)
# include_directories( ${FishGUI_INCLUDE_DIR} )
# # set(FishGUI_EXTENSION_DIR ${FishGUI_DIR}/examples/common)
# # include_directories(${FishGUI_EXTENSION_DIR})
# # FILE(GLOB_RECURSE EXTENSIONS ${FishGUI_EXTENSION_DIR}/*.hpp ${FishGUI_EXTENSION_DIR}/*.cpp)
# # source_group("extensions" FILES ${EXTENSIONS})
# add_library(FishGUI ${SRCS} ${HEADERS})

# file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/Include/FishEngine/*.hpp ${CMAKE_CURRENT_LIST_DIR}/Include/FishEngine/*.inl)
# file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_LIST_DIR}/Source/FishEngine/*.cpp)
# add_library(FishEngine ${HEADERS} ${SRCS})

# file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor/*.hpp ${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor/*.inl ${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor/*.hpp)
# file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor/*.cpp)
# add_library(FishEditor ${HEADERS} ${SRCS})


# file(GLOB_RECURSE SRCS ./Source/Editor/*.cpp ./Source/Editor/*.hpp)
# add_executable(Editor ${SRCS})
# target_link_libraries(Editor FishEngine)
# target_link_libraries(Editor FishEditor)
# target_link_libraries(Editor FishGUI)
# target_link_libraries(Editor nanovg)
# target_link_libraries(Editor glfw)
# target_link_libraries(Editor ${FBXSDK_LIB})
# target_link_libraries(Editor pybind11::embed)
# target_link_libraries(Editor ${FOLLY_LIBRARIES})
# target_link_libraries(Editor ${Boost_LIBRARIES})
# target_link_libraries(Editor ${PhysXSDK_LIBRARIES})
# target_link_libraries(Editor yaml-cpp)
# target_link_libraries(Editor ${BGFX_LIB})
# if (MSVC)
# 	target_link_libraries(Editor glew_s)
# else()
# 	set_target_properties(Editor PROPERTIES LINK_FLAGS "-framework OpenGL")

# 	SET( INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/MacOSXBundleInfo.plist.in)
# 	IF (NOT EXISTS ${INFO_PLIST})
# 		MESSAGE(FATAL_ERROR "${INFO_PLIST} not found!")
# 	ENDIF()
# 	set_target_properties(Editor PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST}" )
# endif()

# add_subdirectory(./Test)


# AutoGroup(${CMAKE_CURRENT_LIST_DIR}/Include/FishEngine Util Math Render Internal Component UI System Physics Serialization Animation)
# AutoGroup(${CMAKE_CURRENT_LIST_DIR}/Source/FishEngine Math Render Component UI System Physics Serialization Animation)
# AutoGroup(${CMAKE_CURRENT_LIST_DIR}/Include/FishEditor Serialization)
# AutoGroup(${CMAKE_CURRENT_LIST_DIR}/Source/FishEditor Serialization)

