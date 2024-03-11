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
}

void c_code_backend::emit(GIFBlock* block)
{
	std::string buffer = fmt::format("u64 {1}_data_size = {0};\n"
		"u64 {1}_data[] __attribute__((aligned(16))) = {{\n\t"
		"GIF_SET_TAG({2},1,0,0,0,1),GIF_REG_AD,\n\t", (block->registers.size() + 1) * 16, block->name, block->registers.size());
	fmt::print("Emitting block: {}\n", block->name);
	for(auto& reg : block->registers)
	{
		buffer += dispatch_table[reg->GetID()](this, reg);
		buffer += "\n\t";
	}

	buffer.pop_back();
	buffer.pop_back();
	buffer += "\n};\n";
	fmt::print("{}", buffer);
}


std::string c_code_backend::emit_primitive(c_code_backend* inst, GifRegister* reg)
{
	PRIM *prim = dynamic_cast<PRIM*>(reg);
	if(inst->emit_mode == EmitMode::USE_DEFS)
	{
		return fmt::format("GS_SET_PRIM({},{},0,0,0,{},0,0,0),GS_REG_PRIM,",
			PrimTypeStrings[prim->GetType()],
			prim->IsGouraud() ? "GS_ENABLE" : "GS_DISABLE",
			prim->IsAA1() ? "GS_ENABLE" : "GS_DISABLE");
	}
	else
	{
		return fmt::format("GS_SET_PRIM({},{:d},0,0,0,{:d},0,0,0),0x00,",
			static_cast<int>(prim->GetType()),
			prim->IsGouraud(),
			prim->IsAA1());
	}
}

std::string c_code_backend::emit_rgbaq(c_code_backend* inst, GifRegister* reg)
{
	RGBAQ *rgbaq = dynamic_cast<RGBAQ*>(reg);

	auto val = rgbaq->GetValue();

	return fmt::format("GS_SET_RGBAQ(0x{:02x},0x{:02x},0x{:02x},0x{:02x},0x{:02x}),{},",
			val.i_x, val.i_y, val.i_z, val.i_w, 0, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_RGBAQ" : "0x01");

}

std::string c_code_backend::emit_xyz2(c_code_backend* inst, GifRegister* reg)
{
	XYZ2 *xyz2 = dynamic_cast<XYZ2*>(reg);

	auto val = xyz2->GetValue();
	
	return fmt::format("GS_SET_XYZ({}<<4,{}<<4,{}),{},",
		val.i_x, val.i_y, val.i_z, inst->emit_mode == EmitMode::USE_DEFS ? "GS_REG_XYZ2" : "0x05");
}
