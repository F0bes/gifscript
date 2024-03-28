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
	UV,
	XYZ2,
	TEX0,
	FOG,
	FOGCOL,
	SCISSOR,
	SIGNAL,
	FINISH,
	LABEL,
};

constexpr const char* const GifRegisterStrings[] = {
	"PRIM",
	"RGBAQ",
	"UV",
	"XYZ2",
	"TEX0",
	"FOG",
	"FOGCOL",
	"SCISSOR",
	"SIGNAL",
	"FINISH",
	"LABEL"};


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
	Fogging,
	AA1,
	Texture,

	// TEX0
	CT32,
	CT24,
	CT16,
	Modulate,
	Decal,
	Highlight,
	Highlight2
};

const std::string RegModifierStrings[] = {
	"Point",
	"Line",
	"Triangle",
	"Sprite",
	"Gouraud",
	"Fogging",
	"AA1",
	"Texture",
	"CT32",
	"CT24",
	"CT16"
	"Modulate",
	"Decal",
	"Highlight",
	"Highlight2"};

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
	uint32_t id;
	std::string_view name;
	RAT rat;
	bool sideEffects;

public:
	// Set sideEffects to true if the register should not be considered for dead store elimination
	constexpr GifRegister(uint32_t id, const std::string_view name, RAT rat, bool sideEffects = false)
		: id(id)
		, name(name)
		, rat(rat)
		, sideEffects(sideEffects)
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

	constexpr bool HasSideEffects()
	{
		return sideEffects;
	}

	constexpr std::string_view GetName()
	{
		return name;
	}

	virtual bool Ready() { return false; };

	virtual void Push(uint32_t) = 0;
	virtual void Push(Vec2) = 0;
	virtual void Push(Vec3) = 0;
	virtual void Push(Vec4) = 0;

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

static std::map<PrimType, const char*> PrimTypeStrings = {
	{PrimType::Point, "GS_PRIM_POINT"},
	{PrimType::Line, "GS_PRIM_LINE"},
	{PrimType::LineStrip, "GS_PRIM_LINE_STRIP"},
	{PrimType::Triangle, "GS_PRIM_TRIANGLE"},
	{PrimType::TriangleStrip, "GS_PRIM_TRIANGLE_STRIP"},
	{PrimType::TriangleFan, "GS_PRIM_TRIANGLE_FAN"},
	{PrimType::Sprite, "GS_PRIM_SPRITE"}};

struct PRIM : public GifRegister
{
	std::optional<PrimType> type;
	bool gouraud = false;
	bool aa1 = false;
	bool fogging = false;
	bool texture = false;

public:
	PRIM()
		: GifRegister(0x00, "PRIM", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return type.has_value();
	}

	void Push(uint32_t) override
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
			case Fogging:
				fogging = true;
				break;
			case Texture:
				texture = true;
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

	bool IsFogging()
	{
		return fogging;
	}

	bool IsAA1()
	{
		return aa1;
	}

	bool IsTextured()
	{
		return texture;
	}
};

struct RGBAQ : public GifRegister
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

	void Push(uint32_t) override
	{
		std::cout << "RGBAQ::Push(uint32_t) Not supported!" << std::endl;
	}

	void Push(Vec2) override
	{
		std::cout << "RGBAQ::Push(Vec2) Not supported!" << std::endl;
	}

	void Push(Vec3 v3) override
	{
		value = Vec4(v3.x, v3.y, v3.z, 0xff);
	}

	void Push(Vec4 v4) override
	{
		std::cout << "!!!! RGBAQ PUSHED " << v4.x << " " << v4.y << " " << v4.z << " " << v4.w << std::endl;
		value = v4;
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

struct UV : public GifRegister
{
	std::optional<Vec2> value;

public:
	UV()
		: GifRegister(0x03, "UV", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(uint32_t) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec2 v2) override
	{
		value = v2;
	}

	void Push(Vec3 v3) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec4 v4) override
	{
		logger::error("Not supported!");
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec2 GetValue()
	{
		return value.value();
	}
};

struct XYZ2 : public GifRegister
{
	std::optional<Vec3> value;

	XYZ2()
		: GifRegister(0x05, "XYZ2", RAT::ADP, true)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(uint32_t) override
	{
		std::cerr << "XYZ2::Push(uint32_t) Not supported!" << std::endl;
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

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec3 GetValue()
	{
		return value.value();
	}
};

enum class PSM
{
	CT32,
	CT24,
	CT16
};

enum class TFX
{
	Modulate,
	Decal,
	Highlight,
	Highlight2
};

struct TEX0 : public GifRegister
{
	std::optional<uint32_t> tbp;
	std::optional<uint32_t> tbw;
	std::optional<PSM> psm;
	std::optional<uint32_t> tw;
	std::optional<uint32_t> th;
	TFX tfx = TFX::Decal;
	bool tcc = 0;

	TEX0()
		: GifRegister(0x06, "TEX0", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return tbp.has_value() && tbw.has_value() && psm.has_value() && tw.has_value() && th.has_value();
	}

	void Push(uint32_t i) override
	{
		if (!tbp.has_value())
			tbp = i;
		else if (!tbw.has_value())
			tbw = i;
		else if (!tw.has_value())
			tw = i;
		else if (!th.has_value())
			th = i;
		else
			logger::error("Unsure what you're trying to push to TEX0 (%d)", i);
	}

	void Push(Vec2 v2) override
	{
		if (!tw.has_value() && !th.has_value())
		{
			tw = v2.x;
			th = v2.y;
		}
		else
		{
			logger::error("Unsure what you're trying to push to TEX0 (%d, %d)", v2.x, v2.y);
		}
	}

	void Push(Vec3 v3) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec4) override
	{
		logger::error("Not supported!");
	}

	bool ApplyModifier(RegModifier mod) override
	{
		switch (mod)
		{
			case CT32:
				psm = PSM::CT32;
				break;
			case CT24:
				psm = PSM::CT24;
				break;
			case CT16:
				psm = PSM::CT16;
				break;
			case Modulate:
				tfx = TFX::Modulate;
				break;
			case Decal:
				tfx = TFX::Decal;
				break;
			case Highlight:
				tfx = TFX::Highlight;
				break;
			case Highlight2:
				tfx = TFX::Highlight2;
				break;
			default:
				logger::error("Unknown modifier: %d", mod);
				return false;
		}

		return true;
	}

	uint32_t GetTBP()
	{
		return tbp.value();
	}

	uint32_t GetTBW()
	{
		return tbw.value();
	}

	PSM GetPSM()
	{
		return psm.value();
	}

	uint32_t GetTW()
	{
		return tw.value();
	}

	uint32_t GetTH()
	{
		return th.value();
	}

	bool GetTCC()
	{
		return tcc;
	}

	TFX GetTFX()
	{
		return tfx;
	}
};

class FOG : public GifRegister
{
	std::optional<uint8_t> value;

public:
	FOG()
		: GifRegister(0x0A, "FOG", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(uint32_t i) override
	{
		value = i;
	}

	void Push(Vec2) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec3 v3) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec4) override
	{
		logger::error("Not supported!");
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	uint8_t GetValue()
	{
		return value.value();
	}
};

class FOGCOL : public GifRegister
{
	std::optional<Vec3> value;

public:
	FOGCOL()
		: GifRegister(0x3D, "FOGCOL", RAT::ADP)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(uint32_t) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec2) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec3 v3) override
	{
		value = v3;
	}

	void Push(Vec4) override
	{
		logger::error("Not supported!");
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

class SCISSOR : public GifRegister
{
	std::optional<Vec4> value;

public:
	SCISSOR()
		: GifRegister(0x40, "SCISSOR", RAT::AD)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	void Push(uint32_t) override
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

	void Push(Vec4 v4) override
	{
		value = v4;
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

class SIGNAL : public GifRegister
{
	std::optional<Vec2> value;

public:
	SIGNAL()
		: GifRegister(0x60, "SIGNAL", RAT::AD, true)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	// Assume that all bits are not masked
	void Push(uint32_t i) override
	{
		value = Vec2(i, ~0u);
	}

	void Push(Vec2 v2) override
	{
		value = v2;
	}

	void Push(Vec3) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec4 v4) override
	{
		logger::error("Not supported!");
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec2 GetValue()
	{
		return value.value();
	}
};

class FINISH : public GifRegister
{
	// The value is not used by the PS2
	// But I will allow it to be set
	uint32_t value = 0;

public:
	FINISH()
		: GifRegister(0x61, "FINISH", RAT::AD, true)
	{
	}

	bool Ready() override
	{
		return true;
	}

	void Push(uint32_t i) override
	{
		value = i;
	}

	void Push(Vec2) override
	{
		logger::warn("Non-Integer value pushed to FINISH register, ignored.");
	}

	void Push(Vec3) override
	{
		logger::warn("Non-Integer value pushed to FINISH register, ignored.");
	}

	void Push(Vec4 v4) override
	{
		logger::warn("Non-Integer value pushed to FINISH register, ignored.");
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	uint32_t GetValue()
	{
		return value;
	}
};

class LABEL : public GifRegister
{
	std::optional<Vec2> value;

public:
	LABEL()
		: GifRegister(0x62, "LABEL", RAT::AD, true)
	{
	}

	bool Ready() override
	{
		return value.has_value();
	}

	// Assume that all bits are not masked
	void Push(uint32_t i) override
	{
		value = Vec2{i, ~0u};
	}

	void Push(Vec2 v2) override
	{
		value = v2;
	}

	void Push(Vec3) override
	{
		logger::error("Not supported!");
	}

	void Push(Vec4 v4) override
	{
		logger::error("Not supported!");
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec2 GetValue()
	{
		return value.value();
	}
};

struct GIFBlock
{
	std::string name;
	std::shared_ptr<PRIM> prim = nullptr;
	std::list<std::shared_ptr<GifRegister>> registers;

	GIFBlock(const std::string name)
		: name(name)
	{
	}

	GIFBlock() = default;
};

// gross
std::shared_ptr<GifRegister> GenReg(GifRegisters reg);

constexpr const char* GetRegString(GifRegisters reg)
{
	return GifRegisterStrings[(int)reg];
};
