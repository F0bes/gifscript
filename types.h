#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <fmt/format.h>

static std::vector<std::string> vec_split(const std::string& s)
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
	return parts;
}

static uint32_t vec_parse_segment(const std::string& s)
{
	if(s.size() > 2 && s[0] == '0' && s[1] == 'x')
	{
		return std::stoul(s, nullptr, 16);
	}
	else if(s.find('.') != std::string::npos)
	{
		return std::bit_cast<uint32_t>(std::stof(s));
	}
	else
	{
		return std::stoul(s);
	}
}

struct Vec2
{
	uint32_t x, y;

	static Vec2 Parse(const std::string& s)
	{
		Vec2 v2;
		auto parts = vec_split(s);
		v2.x = vec_parse_segment(parts.at(0));
		v2.y = vec_parse_segment(parts.at(1));
		return v2;
	}
};

struct Vec3
{
	uint32_t x, y, z;

	static Vec3 Parse(const std::string& s)
	{
		Vec3 v3;
		auto parts = vec_split(s);
		v3.x = vec_parse_segment(parts.at(0));
		v3.y = vec_parse_segment(parts.at(1));
		v3.z = vec_parse_segment(parts.at(2));
		return v3;
	}
};

struct Vec4
{
	uint32_t x, y, z, w;

	static Vec4 Parse(const std::string& s)
	{
		Vec4 v4;
		auto parts = vec_split(s);
		v4.x = vec_parse_segment(parts.at(0));
		v4.y = vec_parse_segment(parts.at(1));
		v4.z = vec_parse_segment(parts.at(2));
		v4.w = vec_parse_segment(parts.at(3));
		return v4;
	}
};
