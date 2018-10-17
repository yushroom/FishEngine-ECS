#pragma once

template<class T>
class EnumFlags
{
public:
	//using B = std::underlying_type_t<T>;
	using B = uint32_t;
	static_assert(std::is_enum_v<T>, "T must be enum");

	EnumFlags() = default;
	EnumFlags(T flag)
	{
		m_flags = static_cast<B>(flag);
	}

	// set the flag
	EnumFlags<T> operator|(EnumFlags<T> flag)
	{
		auto f = *this;
		f |= flag;
		return f;
	}

	// check if the flag is set
	bool operator&(T flag)
	{
		return (m_flags & static_cast<B>(flag)) != 0;
	}

	void operator|=(EnumFlags<T> flag)
	{
		m_flags |= flag.m_flags;
	}

private:
	B m_flags = 0;
};

#define GEN_ENUM_FLAGS(T) \
	typedef EnumFlags<T> T##Flags;	\
	inline T##Flags operator|(T a, T b) { return T##Flags(a) | b; }