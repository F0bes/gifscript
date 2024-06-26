#pragma once

#include "types.hpp"
#include "logger.hpp"
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

enum class GifRegisterID
{
	PRIM = 0x00,
	RGBAQ = 0x01,
	UV = 0x03,
	XYZ2 = 0x05,
	TEX0 = 0x06,
	FOG = 0x0A,
	FOGCOL = 0x3D,
	SCISSOR = 0x40,
	SIGNAL = 0x60,
	FINISH = 0x61,
	LABEL = 0x62
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
	GifRegisterID id;
	std::string name;
	RAT rat;
	bool sideEffects;

public:
	// Set sideEffects to true if the register should not be considered for dead store elimination
	constexpr GifRegister(GifRegisterID id, const std::string name, RAT rat, bool sideEffects = false)
		: id(id)
		, name(name)
		, rat(rat)
		, sideEffects(sideEffects)
	{
	}

	virtual ~GifRegister() = default;

	constexpr GifRegisterID GetID()
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

	const std::string GetName()
	{
		return name;
	}

	virtual bool Ready() const noexcept = 0;

	virtual bool Push(uint32_t) = 0;
	virtual bool Push(Vec2) = 0;
	virtual bool Push(Vec3) = 0;
	virtual bool Push(Vec4) = 0;

	virtual bool ApplyModifier(RegModifier) = 0;

	[[nodiscard]] virtual std::unique_ptr<GifRegister> Clone() = 0;
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
		: GifRegister(GifRegisterID::PRIM, "PRIM", RAT::ADP)
	{
	}

	bool Ready() const noexcept override
	{
		return type.has_value();
	}

	bool Push(uint32_t) override
	{
		return false;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3) override
	{
		return false;
	}

	bool Push(Vec4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier mod) override
	{
		switch(mod)
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
				return false;
		}

		return true;
	}

	PrimType GetType() const noexcept
	{
		return type.value();
	}

	bool IsGouraud() const noexcept
	{
		return gouraud;
	}

	bool IsFogging() const noexcept
	{
		return fogging;
	}

	bool IsAA1() const noexcept
	{
		return aa1;
	}

	bool IsTextured() const noexcept
	{
		return texture;
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

struct RGBAQ : public GifRegister
{
	std::optional<Vec4> value;

public:
	RGBAQ()
		: GifRegister(GifRegisterID::RGBAQ, "RGBAQ", RAT::ADP)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	bool Push(uint32_t) override
	{
		return false;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3 v3) override
	{
		value = Vec4(v3.x, v3.y, v3.z, 0xff);
		return true;
	}

	bool Push(Vec4 v4) override
	{
		std::cout << "!!!! RGBAQ PUSHED " << v4.x << " " << v4.y << " " << v4.z << " " << v4.w << std::endl;
		value = v4;
		return true;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec4 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

struct UV : public GifRegister
{
	std::optional<Vec2> value;

public:
	UV()
		: GifRegister(GifRegisterID::UV, "UV", RAT::ADP)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	bool Push(uint32_t) override
	{
		return false;
	}

	bool Push(Vec2 v2) override
	{
		value = v2;
		return true;
	}

	bool Push(Vec3 v3) override
	{
		return false;
	}

	bool Push(Vec4 v4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec2 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

struct XYZ2 : public GifRegister
{
	std::optional<Vec3> value;

	XYZ2()
		: GifRegister(GifRegisterID::XYZ2, "XYZ2", RAT::ADP, true)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	bool Push(uint32_t) override
	{
		return false;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3 v3) override
	{
		value = v3;
		return true;
	}

	bool Push(Vec4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec3 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
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
		: GifRegister(GifRegisterID::TEX0, "TEX0", RAT::ADP)
	{
	}

	bool Ready() const noexcept override
	{
		return tbp.has_value() && tbw.has_value() && psm.has_value() && tw.has_value() && th.has_value();
	}

	bool Push(uint32_t i) override
	{
		if(!tbp.has_value())
			tbp = i;
		else if(!tbw.has_value())
			tbw = i;
		else if(!tw.has_value())
			tw = i;
		else if(!th.has_value())
			th = i;
		else
			return false;


		return true;
	}

	bool Push(Vec2 v2) override
	{
		if(!tw.has_value() && !th.has_value())
		{
			tw = v2.x;
			th = v2.y;
			return true;
		}

		return false;
	}

	bool Push(Vec3 v3) override
	{
		return false;
	}

	bool Push(Vec4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier mod) override
	{
		switch(mod)
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

	uint32_t GetTBP() const noexcept
	{
		return tbp.value();
	}

	uint32_t GetTBW() const noexcept
	{
		return tbw.value();
	}

	PSM GetPSM() const noexcept
	{
		return psm.value();
	}

	uint32_t GetTW() const noexcept
	{
		return tw.value();
	}

	uint32_t GetTH() const noexcept
	{
		return th.value();
	}

	bool GetTCC() const noexcept
	{
		return tcc;
	}

	TFX GetTFX() const noexcept
	{
		return tfx;
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

class FOG : public GifRegister
{
	std::optional<uint8_t> value;

public:
	FOG()
		: GifRegister(GifRegisterID::FOG, "FOG", RAT::ADP)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	bool Push(uint32_t i) override
	{
		value = i;
		return true;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3 v3) override
	{
		return false;
	}

	bool Push(Vec4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	uint8_t GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

class FOGCOL : public GifRegister
{
	std::optional<Vec3> value;

public:
	FOGCOL()
		: GifRegister(GifRegisterID::FOGCOL, "FOGCOL", RAT::ADP)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	bool Push(uint32_t) override
	{
		return false;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3 v3) override
	{
		value = v3;
		return true;
	}

	bool Push(Vec4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec3 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

class SCISSOR : public GifRegister
{
	std::optional<Vec4> value;

public:
	SCISSOR()
		: GifRegister(GifRegisterID::SCISSOR, "SCISSOR", RAT::AD)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	bool Push(uint32_t) override
	{
		return false;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3) override
	{
		return false;
	}

	bool Push(Vec4 v4) override
	{
		value = v4;
		return true;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec4 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

class SIGNAL : public GifRegister
{
	std::optional<Vec2> value;

public:
	SIGNAL()
		: GifRegister(GifRegisterID::SIGNAL, "SIGNAL", RAT::AD, true)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	// Assume that all bits are not masked
	bool Push(uint32_t i) override
	{
		value = Vec2(i, ~0u);
		return true;
	}

	bool Push(Vec2 v2) override
	{
		value = v2;
		return true;
	}

	bool Push(Vec3) override
	{
		return false;
	}

	bool Push(Vec4 v4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec2 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

class FINISH : public GifRegister
{
	// The value is not used by the PS2
	// But I will allow it to be set
	uint32_t value = 0;

public:
	FINISH()
		: GifRegister(GifRegisterID::FINISH, "FINISH", RAT::AD, true)
	{
	}

	bool Ready() const noexcept override
	{
		return true;
	}

	bool Push(uint32_t i) override
	{
		value = i;
		return true;
	}

	bool Push(Vec2) override
	{
		return false;
	}

	bool Push(Vec3) override
	{
		return false;
	}

	bool Push(Vec4 v4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	uint32_t GetValue() const noexcept
	{
		return value;
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

class LABEL : public GifRegister
{
	std::optional<Vec2> value;

public:
	LABEL()
		: GifRegister(GifRegisterID::LABEL, "LABEL", RAT::AD, true)
	{
	}

	bool Ready() const noexcept override
	{
		return value.has_value();
	}

	// Assume that all bits are not masked
	bool Push(uint32_t i) override
	{
		value = Vec2{i, ~0u};
		return true;
	}

	bool Push(Vec2 v2) override
	{
		value = v2;
		return true;
	}

	bool Push(Vec3) override
	{
		return false;
	}

	bool Push(Vec4 v4) override
	{
		return false;
	}

	bool ApplyModifier(RegModifier) override
	{
		return false;
	}

	Vec2 GetValue() const noexcept
	{
		return value.value();
	}

	std::unique_ptr<GifRegister> Clone() override
	{
		return std::make_unique<std::decay_t<decltype(*this)>>(*this);
	}
};

struct GIFBlock
{
	std::string name;
	std::unique_ptr<GifRegister> prim;
	std::list<std::unique_ptr<GifRegister>> registers;

	GIFBlock(const std::string name)
		: name(name)
	{
	}

	GIFBlock() = default;

	GIFBlock(const GIFBlock& src)
	{
		this->name = src.name;
		if(src.prim)
		{
			this->prim = src.prim->Clone();
		}

		for(auto& reg : src.registers)
		{
			this->registers.push_back(reg->Clone());
		}
	}
	// Helper functions

	bool HasRegister()
	{
		return !registers.empty();
	}

	GifRegister& CurrentRegister()
	{
		return *registers.back();
	}
};

[[nodiscard]] std::unique_ptr<GifRegister> GenReg(GifRegisters reg);

constexpr const char* GetRegString(GifRegisters reg)
{
	return GifRegisterStrings[(int)reg];
};
