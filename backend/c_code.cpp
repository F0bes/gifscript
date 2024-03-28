#include "c_code.h"
#include "registers.h"
#include <fmt/core.h>
#include <functional>
#include "logger.h"


c_code_backend::c_code_backend() noexcept
{
}

bool c_code_backend::arg_parse(int argc, char** argv)
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

void c_code_backend::emit(GIFBlock* block)
{
	std::string prim_str;
	if(block->prim)
	{
		if(emit_mode == EmitMode::USE_DEFS)
		{
			prim_str = fmt::format("GS_SET_PRIM({},{},{},{},0,{},GS_ENABLE,0,0)", PrimTypeStrings[block->prim->GetType()],
				block->prim->IsGouraud() ? "GS_ENABLE" : "GS_DISABLE",
				block->prim->IsTextured() ? "GS_ENABLE" : "GS_DISABLE",
				block->prim->IsFogging() ? "GS_ENABLE" : "GS_DISABLE",
				block->prim->IsAA1() ? "GS_ENABLE" : "GS_DISABLE");
		}
		else
		{
			prim_str = fmt::format("GS_SET_PRIM({},{:d},{:d},{:d},0,{:d},1,0,0)", static_cast<int>(block->prim->GetType()),
				block->prim->IsGouraud(),
				block->prim->IsTextured(),
				block->prim->IsFogging(),
				block->prim->IsAA1());
		}
	}

	std::string buffer = fmt::format("u64 {1}_data_size = {0};\n"
									 "u64 {1}_data[] __attribute__((aligned(16))) = {{\n\t"
									 "GIF_SET_TAG({2},1,{3},{4},0,1),GIF_REG_AD,\n\t",
		(block->registers.size() + 1) * 16, block->name, block->registers.size(), block->prim ? 1 : 0, block->prim ? prim_str : "0");
	fmt::print("Emitting block: {}\n", block->name);
	for(auto& reg : block->registers)
	{
		buffer += dispatch_table[reg->GetID()](this, reg);
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
			if(file == nullptr)
			{
				logger::error("Failed to open file: %s\n", output.cbegin());
				return;
			}
		}
		const std::string prologue = "#include <tamtypes.h>\n#include <gs_gp.h>\n#include <gif_tags.h>\n";
		fwrite(prologue.c_str(), 1, prologue.size(), file);
	}

	fwrite(buffer.c_str(), 1, buffer.size(), file);
}


std::string c_code_backend::emit_primitive(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	PRIM prim = dynamic_cast<PRIM&>(*reg);
	if(inst->emit_mode == EmitMode::USE_DEFS)
	{
		return fmt::format("GS_SET_PRIM({},{},{},{},0,{},GS_ENABLE,0,0),GS_REG_PRIM,",
			PrimTypeStrings[prim.GetType()],
			prim.IsGouraud() ? "GS_ENABLE" : "GS_DISABLE",
			prim.IsTextured() ? "GS_ENABLE" : "GS_DISABLE",
			prim.IsFogging() ? "GS_ENABLE" : "GS_DISABLE",
			prim.IsAA1() ? "GS_ENABLE" : "GS_DISABLE");
	}
	else
	{
		return fmt::format("GS_SET_PRIM({},{:d},{:d},{:d},0,{:d},1,0,0),0x00,",
			static_cast<int>(prim.GetType()),
			prim.IsGouraud(),
			prim.IsTextured(),
			prim.IsFogging(),
			prim.IsAA1());
	}
}

std::string c_code_backend::emit_rgbaq(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	RGBAQ rgbaq = dynamic_cast<RGBAQ&>(*reg);

	auto val = rgbaq.GetValue();

	return fmt::format("GS_SET_RGBAQ(0x{:02x},0x{:02x},0x{:02x},0x{:02x},0x{:02x}),{},",
		val.x, val.y, val.z, val.w, 0, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_RGBAQ" : "0x01");
}

std::string c_code_backend::emit_uv(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	UV uv = dynamic_cast<UV&>(*reg);

	auto val = uv.GetValue();

	return fmt::format("GS_SET_UV({}<<4,{}<<4),{},",
		val.x, val.y, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_UV" : "0x03");
}

std::string c_code_backend::emit_xyz2(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	XYZ2 xyz2 = dynamic_cast<XYZ2&>(*reg);

	auto val = xyz2.GetValue();

	return fmt::format("GS_SET_XYZ({}<<4,{}<<4,{}),{},",
		val.x, val.y, val.z, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_XYZ2" : "0x05");
}

std::string c_code_backend::emit_tex0(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	TEX0 tex0 = dynamic_cast<TEX0&>(*reg);

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
	else
	{
		return fmt::format("GS_SET_TEX0(0x{:02x},0x{:02x},{},{:02x},{:02x},{:d},{},0,0,0,0,0),0x06,",
			tex0.GetTBP(), tex0.GetTBW(), static_cast<uint32_t>(tex0.GetPSM()),
			tex0.GetTW(), tex0.GetTH(), tex0.GetTCC(), static_cast<uint32_t>(tex0.GetTFX()));
	}
}

std::string c_code_backend::emit_fog(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	FOG fog = dynamic_cast<FOG&>(*reg);

	auto val = fog.GetValue();

	return fmt::format("GS_SET_FOG(0x{:02x}),{},",
		val, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_FOG" : "0x0A");
}

std::string c_code_backend::emit_fogcol(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	FOGCOL fogcol = dynamic_cast<FOGCOL&>(*reg);

	auto val = fogcol.GetValue();

	return fmt::format("GS_SET_FOGCOL(0x{:02x},0x{:02x},0x{:02x}),{},",
		val.x, val.y, val.z, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_FOGCOL" : "0x3D");
}

std::string c_code_backend::emit_scissor(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	SCISSOR scissor = dynamic_cast<SCISSOR&>(*reg);

	auto val = scissor.GetValue();

	return fmt::format("GS_SET_SCISSOR({},{},{},{}),{},",
		val.x, val.y, val.z, val.w, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_SCISSOR" : "0x40");
}

std::string c_code_backend::emit_signal(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	SIGNAL signal = dynamic_cast<SIGNAL&>(*reg);

	auto val = signal.GetValue();

	return fmt::format("GS_SET_SIGNAL(0x{:02x},0x{:02x}),{},",
		val.x, val.y, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_SIGNAL" : "0x60");
}

std::string c_code_backend::emit_finish(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	FINISH finish = dynamic_cast<FINISH&>(*reg);

	auto val = finish.GetValue();
	if(inst->emit_mode == EmitMode::USE_DEFS)
	{
		return fmt::format("GS_SET_FINISH({}),GS_REG_FINISH,", val);
	}
	else
	{
		return fmt::format("0x{:x},{},", val, "0x61");
	}
}

std::string c_code_backend::emit_label(c_code_backend* inst, std::shared_ptr<GifRegister> reg)
{
	LABEL label = dynamic_cast<LABEL&>(*reg);

	auto val = label.GetValue();

	return fmt::format("GS_SET_LABEL(0x{:02x},0x{:02x}),{},",
		val.x, val.y, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_LABEL" : "0x62");
}
