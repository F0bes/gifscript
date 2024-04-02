#include "gifscript_backend.hpp"
#include "registers.hpp"
#include "logger.hpp"
#include "version.hpp"

#include <fmt/core.h>
#include <functional>

auto gifscript_backend::arg_parse(int argc, char** argv) -> bool
{
	return true;
}

void gifscript_backend::print_help() const
{
	fmt::print(
		"gifscript backend options: none\n");
}

gifscript_backend::~gifscript_backend()
{
	if(file != nullptr && file != stdout)
	{
		fclose(file);
	}
}

void gifscript_backend::emit(GIFBlock& block)
{
	std::string buffer = fmt::format("{} {{\n\t", block.name);
	fmt::print("Emitting block: {}\n", block.name);
	for(const auto& reg : block.registers)
	{
		buffer += dispatch_table[static_cast<uint32_t>(reg->GetID())](this, *reg);
		buffer += "\n\t";
	}

	buffer.pop_back();
	buffer.pop_back();
	buffer += "\n}\n";

	if(first_emit)
	{
		if(output.empty())
		{
			file = stdout;
		}
		else
		{
			file = fopen(output.c_str(), "w");
		}

		if(file == nullptr)
		{
			logger::error("Failed to open file: %s\n", output.cbegin());
			return;
		}
		const std::string prologue = fmt::format("// Generated with GIFScript version {}\n", GIT_VERSION);
		fwrite(prologue.c_str(), 1, prologue.size(), file);
	}

	fwrite(buffer.c_str(), 1, buffer.size(), file);
}

auto gifscript_backend::emit_primitive(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& prim = dynamic_cast<const PRIM&>(reg);

	std::string line = "prim ";
	switch(prim.GetType())
	{
		case PrimType::Point:
			line += "point";
			break;
		case PrimType::Line:
			line += "line";
			break;
		case PrimType::LineStrip:
			line += "linestrip";
			break;
		case PrimType::Triangle:
			line += "triangle";
			break;
		case PrimType::TriangleStrip:
			line += "trianglestrip";
			break;
		case PrimType::TriangleFan:
			line += "trianglefan";
			break;
		case PrimType::Sprite:
			line += "sprite";
			break;
		default:
			logger::error("Unknown primitive type: %d\n", static_cast<int>(prim.GetType()));
	}

	if(prim.IsGouraud())
	{
		line += " gouraud";
	}

	if(prim.IsTextured())
	{
		line += " textured";
	}

	if(prim.IsFogging())
	{
		line += " fogging";
	}

	if(prim.IsAA1())
	{
		line += " aa1";
	}

	line += ";";
	return line;
}

auto gifscript_backend::emit_rgbaq(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& rgbaq = dynamic_cast<const RGBAQ&>(reg);

	auto val = rgbaq.GetValue();
	return fmt::format("rgbaq 0x{:x},0x{:x},0x{:x},0x{:x};",
		val.x, val.y, val.z, val.w);
}

auto gifscript_backend::emit_uv(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& uv_reg = dynamic_cast<const UV&>(reg);

	auto val = uv_reg.GetValue();
	return fmt::format("uv 0x{:x},0x{:x};",
		val.x, val.y);
}

auto gifscript_backend::emit_xyz2(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& xyz2 = dynamic_cast<const XYZ2&>(reg);

	auto val = xyz2.GetValue();
	return fmt::format("xyz2 0x{:x},0x{:x},0x{:x};",
		val.x, val.y, val.z);
}

auto gifscript_backend::emit_tex0(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& tex0 = dynamic_cast<const TEX0&>(reg);

	std::string line = fmt::format("tex0 0x{:x} 0x{:x} 0x{:x},0x{:x}",
		tex0.GetTBP(), tex0.GetTBW(), tex0.GetTW(), tex0.GetTH());

	switch(tex0.GetPSM())
	{
		case PSM::CT32:
			line += " CT32";
			break;
		case PSM::CT24:
			line += " CT24";
			break;
		case PSM::CT16:
			line += " CT16";
			break;
	}

	switch(tex0.GetTFX())
	{
		case TFX::Modulate:
			line += " modulate";
			break;
		case TFX::Decal:
			line += " decal";
			break;
		case TFX::Highlight:
			line += " highlight";
			break;
		case TFX::Highlight2:
			line += " highlight2";
			break;
	}

	line += ";";
	return line;
}

auto gifscript_backend::emit_fog(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& fog = dynamic_cast<const FOG&>(reg);

	auto val = fog.GetValue();
	return fmt::format("fog 0x{:x};",
		val);
}

auto gifscript_backend::emit_fogcol(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& fogcol = dynamic_cast<const FOGCOL&>(reg);

	auto val = fogcol.GetValue();
	return fmt::format("fogcol 0x{:x},0x{:x},0x{:x};",
		val.x, val.y, val.z);
}

auto gifscript_backend::emit_scissor(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& scissor = dynamic_cast<const SCISSOR&>(reg);

	auto val = scissor.GetValue();
	return fmt::format("scissor 0x{:x},0x{:x},0x{:x},0x{:x};",
		val.x, val.y, val.z, val.w);
}

auto gifscript_backend::emit_signal(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& signal = dynamic_cast<const SIGNAL&>(reg);

	auto val = signal.GetValue();
	return fmt::format("signal 0x{:x},0x{:x};",
		val.x, val.y);
}

auto gifscript_backend::emit_finish(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& finish = dynamic_cast<const FINISH&>(reg);

	auto val = finish.GetValue();
	return fmt::format("finish 0x{:x};",
		val);
}

auto gifscript_backend::emit_label(gifscript_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& label = dynamic_cast<const LABEL&>(reg);

	auto val = label.GetValue();
	return fmt::format("label 0x{:x},0x{:x};",
		val.x, val.y);
}
