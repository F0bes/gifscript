#pragma once

#include "types.h"
#include "logger.h"
#include <optional>
#include <iostream>
#include <algorithm>
#include <memory>
#include <fmt/core.h>
#include <list>
#include <map>

enum class GifRegisters
{
	PRIM,
	RGBAQ,
	XYZ2
};

const char *const GifRegisterStrings[] = {
	"PRIM",
	"RGBAQ",
	"XYZ2"};


enum RegModifier : uint32_t
{
	// PRIM
	Point = 1,
	Line,
	LineStrip,
	Triangle,
	TriangleStrip,
	TriangleFan,
	Sprite,
	Gouraud,
	AA1
};

const std::string RegModifierStrings[] = {
	"Point",
	"Line",
	"Triangle",
	"Sprite",
	"Gouraud",
	"AA1"};

// Register Access Type
// Used to determine if the gif block can use packed or reglist modes
enum class RAT
{
	// Can only use AD
	AD,
	// Can use AD or Packed
	ADP
};

class GifRegister
{
	RAT rat;
	std::string_view name;
	uint32_t id;

public:
	constexpr GifRegister(uint32_t id, const std::string_view name, RAT rat)
		: id(id), name(name), rat(rat)
	{
	}

	virtual ~GifRegister() = default;

	constexpr uint32_t GetID()
	{
		return id;
	}

	constexpr bool RequiresAD()
	{
		return rat == RAT::AD;
	}

	constexpr std::string_view GetName()
	{
		return name;
	}

	virtual bool Ready() { return false; };

	virtual void Push(int32_t) = 0;
	virtual void Push(Vec2) = 0;
	virtual void Push(Vec3) = 0;
	virtual void Push(Vec4) = 0;

	virtual void Pushf(int32_t) = 0;
	virtual void Pushf(Vec2) = 0;
	virtual void Pushf(Vec3) = 0;
	virtual void Pushf(Vec4) = 0;

	virtual bool ApplyModifier(RegModifier) = 0;
};

enum class PrimType
{
	Point,
	Line,
	LineStrip,
	Triangle,
	TriangleStrip,
	TriangleFan,
	Sprite
};

static std::map<PrimType,const char*> PrimTypeStrings = {
	{PrimType::Point, "GS_PRIM_POINT"},
	{PrimType::Line, "GS_PRIM_LINE"},
	{PrimType::LineStrip, "GS_PRIM_LINE_STRIP"},
	{PrimType::Triangle, "GS_PRIM_TRIANGLE"},
	{PrimType::TriangleStrip, "GS_PRIM_TRIANGLE_STRIP"},
	{PrimType::TriangleFan, "GS_PRIM_TRIANGLE_FAN"},
	{PrimType::Sprite, "GS_PRIM_SPRITE"}};

class PRIM : public GifRegister
{
	std::optional<PrimType> type;
	bool gouraud = false;
	bool aa1 = false;

public:
	PRIM()
		: GifRegister(0x00, "PRIM", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return type.has_value();
	}

	void Push(int32_t) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec2) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec3) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec4) override
	{
		logger::error("Not supported!");
	}

	void Pushf(int32_t) override
	{
	}

	void Pushf(Vec2) override
	{
	}

	void Pushf(Vec3) override
	{
	}

	void Pushf(Vec4) override
	{
	}

	bool ApplyModifier(RegModifier mod) override
	{
		std::cout << "Applying modifier: " << mod << std::endl;
		switch (mod)
		{
		case Point:
			type = PrimType::Point;
			break;
		case Line:
			type = PrimType::Line;
			break;
		case LineStrip:
			type = PrimType::LineStrip;
			break;
		case Triangle:
			type = PrimType::Triangle;
			break;
		case TriangleStrip:
			type = PrimType::TriangleStrip;
			break;
		case TriangleFan:
			type = PrimType::TriangleFan;
			break;
		case Sprite:
			type = PrimType::Sprite;
			break;
		case Gouraud:
			gouraud = true;
			break;
		case AA1:
			aa1 = true;
			break;
		default:
			std::cerr << "Unknown modifier: " << mod << std::endl;
			return false;
		}

		return true;
	}

	PrimType GetType()
	{
		return type.value();
	}

	bool IsGouraud()
	{
		return gouraud;
	}

	bool IsAA1()
	{
		return aa1;
	}
};

class RGBAQ : public GifRegister
{

	std::optional<Vec4> value;

public:
	RGBAQ()
		: GifRegister(0x01, "RGBAQ", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(int32_t) override
	{
		std::cout << "RGBAQ::Push(int32_t) Not supported!" << std::endl;
	}

	void Push(Vec2) override
	{
		std::cout << "RGBAQ::Push(Vec2) Not supported!" << std::endl;
	}

	void Push(Vec3 v3) override
	{
		value = Vec4(v3.i_x, v3.i_y, v3.i_z, 0xff);
	}

	void Push(Vec4 v4) override
	{
		std::cout << "!!!! RGBAQ PUSHED " << v4.i_x << " " << v4.i_y << " " << v4.i_z << " " << v4.i_w << std::endl;
		value = v4;
	}

	void Pushf(int32_t) override
	{
		std::cout << "RGBAQ::Pushf(int32_t) Not supported!" << std::endl;
	}

	void Pushf(Vec2) override
	{
		std::cout << "RGBAQ::Pushf(Vec2) Not supported!" << std::endl;
	}

	void Pushf(Vec3 v3) override
	{
		value = Vec4(v3.i_x, v3.i_y, v3.i_z, 0xff);
		std::cout << "!!!! RGBAQ PUSHED " << v3.i_x << " " << v3.i_y << " " << v3.i_z << " " << 0xff << std::endl;
	}

	void Pushf(Vec4 v4) override
	{
		value = v4.ftoi();
		std::cout << "!!!! RGBAQ PUSHED (4)" << v4.i_x << " " << v4.i_y << " " << v4.i_z << " " << v4.i_w << std::endl;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec4 GetValue()
	{
		return value.value();
	}
};

struct XYZ2 : public GifRegister
{
	std::optional<Vec3> value;

	XYZ2()
		: GifRegister(0x05, "XYZ2", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(int32_t) override
	{
		std::cerr << "XYZ2::Push(int32_t) Not supported!" << std::endl;
	}

	void Push(Vec2) override
	{
		std::cerr << "XYZ2::Push(Vec2) Not supported!" << std::endl;
	}

	void Push(Vec3 v3) override
	{
		value = v3;
	}

	void Push(Vec4) override
	{
		std::cerr << "XYZ2::Push(Vec4) Not supported!" << std::endl;
	}

	void Pushf(int32_t) override
	{
		std::cerr << "XYZ2::Pushf(int32_t) Not supported!" << std::endl;
	}

	void Pushf(Vec2) override
	{
		std::cerr << "XYZ2::Pushf(Vec2) Not supported!" << std::endl;
	}

	void Pushf(Vec3 v3) override
	{
		value = v3.ftoi();
	}

	void Pushf(Vec4) override
	{
		std::cerr << "XYZ2::Pushf(Vec4) Not supported!" << std::endl;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec3 GetValue()
	{
		return value.value();
	}
};

struct GIFBlock
{
	std::string name;
	std::list<std::shared_ptr<GifRegister>> registers;

	GIFBlock(const std::string name)
		: name(name)
	{
	}

	GIFBlock() = default;
};


// gross
static std::shared_ptr<GifRegister> GenReg(GifRegisters reg)
{
	switch (reg)
	{
	case GifRegisters::PRIM:
		return std::make_shared<PRIM>();
	case GifRegisters::RGBAQ:
		return std::make_shared<RGBAQ>();
	case GifRegisters::XYZ2:
		return std::make_shared<XYZ2>();
	}
}

constexpr const char *GetRegString(GifRegisters reg)
{
	return GifRegisterStrings[(int)reg];
};
