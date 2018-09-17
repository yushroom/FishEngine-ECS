#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define FISHENGINE_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define FISHENGINE_PLATFORM_APPLE 1
#else //defined(__linux__)
    #define FISHENGINE_PLATFORM_LINUX 1
#endif

#if defined(FishEngine_SHARED_LIB) && FISHENGINE_PLATFORM_WINDOWS
    #ifdef FishEngine_EXPORTS
        #define FE_EXPORT __declspec(dllexport)
    #else
        #define FE_EXPORT __declspec(dllimport)
    #endif
#else
    #define FE_EXPORT
#endif

#include <string>
#include <cstdint>
#include <vector>

#if defined(_DEBUG) || defined(DEBUG)
#include <cassert>
#define Assert(exp) assert(exp)
#else
#define Assert(exp) (void(0))
#endif

//#define LOGF puts(__FUNCTION__)
#define LOGF
// #define LOGF puts(__func__)

namespace FishEngine
{
	using EntityID = uint32_t;
	class Vector3;
	class Vector4;
	class Ray;
	class Matrix4x4;
}


using String = std::string;
