#pragma once

namespace FishEngine
{
	class Int2
	{
	public:
		int x, y;
	};
		
	class Int3{
	public:
		int x, y, z;
	};
		
	class Int4
	{
	public:
		int x, y, z, w;

		Int4() : x(0), y(0), z(0), w(0) {}
		
		Int4(int x, int y, int z, int w)
			: x(x), y(y), z(z), w(w)
		{
		}
	};
}

