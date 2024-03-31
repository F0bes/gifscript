#include "c_code.h"
#include "registers.h"
#include <fmt/core.h>
#include <functional>
#include "logger.h"

auto c_code_backend::arg_parse(int argc, char** argv) -> bool
{
	for(int i = 0; i < argc; i++)
	{
		const std::string_view arg = argv[i];
		if(arg.starts_with("--b") && !arg.starts_with("--backend="))
		{
			const std::string_view arg2 = arg.substr(3);
			if(arg2.compare("defs") == 0)
			{
				emit_mode = EmitMode::USE_DEFS;
			}
			else if(arg2.compare("magic") == 0)
			{
				emit_mode = EmitMode::USE_MAGIC;
			}
			else if(arg2.compare("help") == 0)
			{
				print_help();
				return false;
			}
			else
			{
				logger::error("Invalid c_code option: %s\n", arg2.cbegin());
				return false;
			}
		}
	}

	return true;
}

void c_code_backend::print_help() const
{
	fmt::print(
		"c_code backend options:\n"
		"\t--bdefs\t\tUse the definitions for registers found in gs_gp.h and gif_tags.h (default)\n"
		"\t--bmagic\tDo not use definitions and emit using raw values. (Still requires gif_tags.h currently)\n");
}

c_code_backend::~c_code_backend()
{
	if(file != nullptr && file != stdout)
	{
		fclose(file);
	}
}

void c_code_backend::emit(GIFBlock& block)
{
	std::string prim_str;
	if(block.prim)
	{
		const auto& prim = dynamic_cast<const PRIM&>(*block.prim);
		if(emit_mode == EmitMode::USE_DEFS)
		{
			prim_str = fmt::format("GS_SET_PRIM({},{},{},{},0,{},GS_ENABLE,0,0)", PrimTypeStrings[prim.GetType()],
				prim.IsGouraud() ? "GS_ENABLE" : "GS_DISABLE",
				prim.IsTextured() ? "GS_ENABLE" : "GS_DISABLE",
				prim.IsFogging() ? "GS_ENABLE" : "GS_DISABLE",
				prim.IsAA1() ? "GS_ENABLE" : "GS_DISABLE");
		}
		else
		{
			prim_str = fmt::format("GS_SET_PRIM({},{:d},{:d},{:d},0,{:d},1,0,0)", static_cast<int>(prim.GetType()),
				prim.IsGouraud(),
				prim.IsTextured(),
				prim.IsFogging(),
				prim.IsAA1());
		}
	}

	const size_t bytes_per_register = 16;
	std::string buffer = fmt::format("u64 {1}_data_size = {0};\n"
									 "u64 {1}_data[] __attribute__((aligned(16))) = {{\n\t"
									 "GIF_SET_TAG({2},1,{3},{4},0,1),GIF_REG_AD,\n\t",
		(block.registers.size() + 1) * bytes_per_register, block.name, block.registers.size(), block.prim ? 1 : 0, block.prim ? prim_str : "0");
	fmt::print("Emitting block: {}\n", block.name);
	for(const auto& reg : block.registers)
	{
		buffer += dispatch_table[static_cast<uint32_t>(reg->GetID())](this, *reg);
		buffer += "\n\t";
	}

	buffer.pop_back();
	buffer.pop_back();
	buffer += "\n};\n";

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
		const std::string prologue = "#include <tamtypes.h>\n#include <gs_gp.h>\n#include <gif_tags.h>\n";
		fwrite(prologue.c_str(), 1, prologue.size(), file);
	}

	fwrite(buffer.c_str(), 1, buffer.size(), file);
}

auto c_code_backend::emit_primitive(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& prim = dynamic_cast<const PRIM&>(reg);
	if(inst->emit_mode == EmitMode::USE_DEFS)
	{
		return fmt::format("GS_SET_PRIM({},{},{},{},0,{},GS_ENABLE,0,0),GS_REG_PRIM,",
			PrimTypeStrings[prim.GetType()],
			prim.IsGouraud() ? "GS_ENABLE" : "GS_DISABLE",
			prim.IsTextured() ? "GS_ENABLE" : "GS_DISABLE",
			prim.IsFogging() ? "GS_ENABLE" : "GS_DISABLE",
			prim.IsAA1() ? "GS_ENABLE" : "GS_DISABLE");
	}

	return fmt::format("GS_SET_PRIM({},{:d},{:d},{:d},0,{:d},1,0,0),0x00,",
		static_cast<int>(prim.GetType()),
		prim.IsGouraud(),
		prim.IsTextured(),
		prim.IsFogging(),
		prim.IsAA1());
}

auto c_code_backend::emit_rgbaq(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& rgbaq = dynamic_cast<const RGBAQ&>(reg);

	auto val = rgbaq.GetValue();

	return fmt::format("GS_SET_RGBAQ(0x{:02x},0x{:02x},0x{:02x},0x{:02x},0x{:02x}),{},",
		val.x, val.y, val.z, val.w, 0, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_RGBAQ" : "0x01");
}

auto c_code_backend::emit_uv(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& uv_reg = dynamic_cast<const UV&>(reg);

	auto val = uv_reg.GetValue();

	return fmt::format("GS_SET_UV({}<<4,{}<<4),{},",
		val.x, val.y, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_UV" : "0x03");
}

auto c_code_backend::emit_xyz2(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& xyz2 = dynamic_cast<const XYZ2&>(reg);

	auto val = xyz2.GetValue();

	return fmt::format("GS_SET_XYZ({}<<4,{}<<4,{}),{},",
		val.x, val.y, val.z, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_XYZ2" : "0x05");
}

auto c_code_backend::emit_tex0(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& tex0 = dynamic_cast<const TEX0&>(reg);

	if(inst->emit_mode == EmitMode::USE_DEFS)
	{
		std::string PSM_STR;
		switch(tex0.GetPSM())
		{
			case PSM::CT32:
				PSM_STR = "GS_PSM_32";
				break;
			case PSM::CT24:
				PSM_STR = "GS_PSM_24";
				break;
			case PSM::CT16:
				PSM_STR = "GS_PSM_16";
				break;
		}

		// Todo: Support gs_psm defines
		return fmt::format("GS_SET_TEX0(0x{:x},0x{:x},{},{:x},{:x},{:d},{},0,0,0,0,0),GS_REG_TEX0,",
			tex0.GetTBP(), tex0.GetTBW(), PSM_STR,
			tex0.GetTW(), tex0.GetTH(), tex0.GetTCC(), static_cast<uint32_t>(tex0.GetTFX()));
	}

	return fmt::format("GS_SET_TEX0(0x{:02x},0x{:02x},{},{:02x},{:02x},{:d},{},0,0,0,0,0),0x06,",
		tex0.GetTBP(), tex0.GetTBW(), static_cast<uint32_t>(tex0.GetPSM()),
		tex0.GetTW(), tex0.GetTH(), tex0.GetTCC(), static_cast<uint32_t>(tex0.GetTFX()));
}

auto c_code_backend::emit_fog(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& fog = dynamic_cast<const FOG&>(reg);

	auto val = fog.GetValue();

	return fmt::format("GS_SET_FOG(0x{:02x}),{},",
		val, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_FOG" : "0x0A");
}

auto c_code_backend::emit_fogcol(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& fogcol = dynamic_cast<const FOGCOL&>(reg);

	auto val = fogcol.GetValue();

	return fmt::format("GS_SET_FOGCOL(0x{:02x},0x{:02x},0x{:02x}),{},",
		val.x, val.y, val.z, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_FOGCOL" : "0x3D");
}

auto c_code_backend::emit_scissor(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& scissor = dynamic_cast<const SCISSOR&>(reg);

	auto val = scissor.GetValue();

	return fmt::format("GS_SET_SCISSOR({},{},{},{}),{},",
		val.x, val.y, val.z, val.w, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_SCISSOR" : "0x40");
}

auto c_code_backend::emit_signal(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& signal = dynamic_cast<const SIGNAL&>(reg);

	auto val = signal.GetValue();

	return fmt::format("GS_SET_SIGNAL(0x{:02x},0x{:02x}),{},",
		val.x, val.y, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_SIGNAL" : "0x60");
}

auto c_code_backend::emit_finish(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& finish = dynamic_cast<const FINISH&>(reg);

	auto val = finish.GetValue();
	if(inst->emit_mode == EmitMode::USE_DEFS)
	{
		return fmt::format("GS_SET_FINISH({}),GS_REG_FINISH,", val);
	}

	return fmt::format("0x{:x},{},", val, "0x61");
}

auto c_code_backend::emit_label(c_code_backend* inst, const GifRegister& reg) -> std::string
{
	const auto& label = dynamic_cast<const LABEL&>(reg);

	auto val = label.GetValue();

	return fmt::format("GS_SET_LABEL(0x{:02x},0x{:02x}),{},",
		val.x, val.y, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_LABEL" : "0x62");
}
