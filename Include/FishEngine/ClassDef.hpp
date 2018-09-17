#pragma once

namespace FishEngine
{

class NonCopyable
{
public:
	NonCopyable() = default;
    NonCopyable(NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = delete;
    NonCopyable& operator=(NonCopyable&) = delete;
};


class Singleton : public NonCopyable
{
protected:
    Singleton() = default;
};


class Static
{
public:
    Static() = delete;
};

}
