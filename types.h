#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <fmt/format.h>

static uint32_t vec_parse_segment(const std::string& s)
{
	if (s.size() > 2 && s[0] == '0' && s[1] == 'x')
	{
		return std::stoul(s, nullptr, 16);
	}
	else if (s.find('.') != std::string::npos)
	{
		return std::bit_cast<uint32_t>(std::stof(s));
	}
	else
	{
		return std::stoul(s);
	}
}

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
		: f_x(x)
		, f_y(y)
	{
	}

	explicit Vec2(uint32_t x, uint32_t y)
		: i_x(x)
		, i_y(y)
	{
	}

	Vec2(const std::string& s)
	{
		std::vector<std::string> parts;
		size_t i, j;
		for(i = 0, j = 0; i < s.size(); i++)
		{
			if(s[i] == ',')
			{
				parts.push_back(s.substr(j, i - j));
				j = i + 1;
			}
		}
		parts.push_back(s.substr(j, i - j));

		i_x = vec_parse_segment(parts.at(0));
		i_y = vec_parse_segment(parts.at(1));
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

	Vec3(const std::string& s)
	{
		std::vector<std::string> parts;
		size_t i, j;
		for(i = 0, j = 0; i < s.size(); i++)
		{
			if(s[i] == ',')
			{
				parts.push_back(s.substr(j, i - j));
				j = i + 1;
			}
		}
		parts.push_back(s.substr(j, i - j));

		i_x = vec_parse_segment(parts.at(0));
		i_y = vec_parse_segment(parts.at(1));
		i_z = vec_parse_segment(parts.at(2));
	}

	explicit Vec3(float x, float y, float z)
		: f_x(x)
		, f_y(y)
		, f_z(z)
	{
	}

	explicit Vec3(uint32_t x, uint32_t y, uint32_t z)
		: i_x(x)
		, i_y(y)
		, i_z(z)
	{
	}

	explicit Vec3(Vec2 v, float z)
		: f_x(v.f_x)
		, f_y(v.f_y)
		, f_z(z)
	{
	}

	explicit Vec3(Vec2 v, uint32_t z)
		: i_x(v.i_x)
		, i_y(v.i_y)
		, i_z(z)
	{
	}

	Vec3 ftoi()
	{
		return Vec3((uint32_t)f_x, (uint32_t)f_y, (uint32_t)f_z);
	}

	std::string toString()
	{
		return fmt::format("{}, {}, {}", i_x, i_y, i_z);
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

	Vec4(const std::string& s)
	{
		std::vector<std::string> parts;
		size_t i, j;
		for(i = 0, j = 0; i < s.size(); i++)
		{
			if(s[i] == ',')
			{
				parts.push_back(s.substr(j, i - j));
				j = i + 1;
			}
		}
		parts.push_back(s.substr(j, i - j));
		
		i_x = vec_parse_segment(parts.at(0));
		i_y = vec_parse_segment(parts.at(1));
		i_z = vec_parse_segment(parts.at(2));
		i_w = vec_parse_segment(parts.at(3));
	}

	explicit Vec4(float x, float y, float z, float w)
		: f_x(x)
		, f_y(y)
		, f_z(z)
		, f_w(w)
	{
	}

	explicit Vec4(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
		: i_x(x)
		, i_y(y)
		, i_z(z)
		, i_w(w)
	{
	}

	explicit Vec4(Vec3 v, float w)
		: f_x(v.f_x)
		, f_y(v.f_y)
		, f_z(v.f_z)
		, f_w(w)
	{
	}

	explicit Vec4(Vec3 v, uint32_t w)
		: i_x(v.i_x)
		, i_y(v.i_y)
		, i_z(v.i_z)
		, i_w(w)
	{
	}

	Vec4 ftoi()
	{
		return Vec4((uint32_t)f_x, (uint32_t)f_y, (uint32_t)f_z, (uint32_t)f_w);
	}

	std::string toString()
	{
		return fmt::format("{}, {}, {}, {}", i_x, i_y, i_z, i_w);
	}
};
