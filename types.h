#pragma once
#include <cstdint>
#include <stdexcept>

union Vec2
{
	struct
	{
		float f_x, f_y;
	};
	struct
	{
		uint32_t i_x, i_y;
	};

	explicit Vec2(float x, float y)
		: f_x(x), f_y(y)
	{}

	explicit Vec2(uint32_t x, uint32_t y)
		: i_x(x), i_y(y)
	{}

	Vec2(const char* s, const char* e)
	{
		if(sscanf(s, "%d,%d", &i_x, &i_y) != 2)
			throw std::runtime_error("Invalid Vec2: " + std::string(s, e));
	}

	Vec2 ftoi()
	{
		return Vec2((uint32_t)f_x, (uint32_t)f_y);
	}
};

union Vec3
{
	struct
	{
		float f_x, f_y, f_z;
	};
	struct
	{
		uint32_t i_x, i_y, i_z;
	};

	Vec3(const char* s, const char* e)
	{
		if(sscanf(s, "%d,%d,%d", &i_x, &i_y, &i_z) != 3)
			throw std::runtime_error("Invalid Vec3: " + std::string(s, e));
	}

	explicit Vec3(float x, float y, float z)
		: f_x(x), f_y(y), f_z(z)
	{}

	explicit Vec3(uint32_t x, uint32_t y, uint32_t z)
		: i_x(x), i_y(y), i_z(z)
	{}

	explicit Vec3(Vec2 v, float z)
		: f_x(v.f_x), f_y(v.f_y), f_z(z)
	{}

	explicit Vec3(Vec2 v, uint32_t z)
		: i_x(v.i_x), i_y(v.i_y), i_z(z)
	{}

	Vec3 ftoi()
	{
		return Vec3((uint32_t)f_x, (uint32_t)f_y, (uint32_t)f_z);
	}

	std::string toString()
	{
		return std::to_string(i_x) + ", " + std::to_string(i_y) + ", " + std::to_string(i_z);
	}
};

union Vec4
{
	struct
	{
		float f_x, f_y, f_z, f_w;
	};
	struct
	{
		uint32_t i_x, i_y, i_z, i_w;
	};

	Vec4() = default;

	Vec4(const char* s, const char* e)
	{
		if(sscanf(s, "%d,%d,%d,%d", &i_x, &i_y, &i_z, &i_w) != 4)
			throw std::runtime_error("Invalid Vec4: " + std::string(s, e));
	}

	explicit Vec4(float x, float y, float z, float w)
		: f_x(x), f_y(y), f_z(z), f_w(w)
	{}

	explicit Vec4(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
		: i_x(x), i_y(y), i_z(z), i_w(w)
	{}

	explicit Vec4(Vec3 v, float w)
		: f_x(v.f_x), f_y(v.f_y), f_z(v.f_z), f_w(w)
	{}

	explicit Vec4(Vec3 v, uint32_t w)
		: i_x(v.i_x), i_y(v.i_y), i_z(v.i_z), i_w(w)
	{}

	Vec4 ftoi()
	{
		return Vec4((uint32_t)f_x, (uint32_t)f_y, (uint32_t)f_z, (uint32_t)f_w);
	}

	std::string toString()
	{
		return std::to_string(i_x) + ", " + std::to_string(i_y) + ", " + std::to_string(i_z) + ", " + std::to_string(i_w);
	}
};
