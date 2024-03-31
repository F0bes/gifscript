#include "registers.h"
#include <utility>

auto GenReg(GifRegisters reg) -> std::unique_ptr<GifRegister>
{
	switch(reg)
	{
		case GifRegisters::PRIM:
			return std::make_unique<PRIM>();
		case GifRegisters::RGBAQ:
			return std::make_unique<RGBAQ>();
		case GifRegisters::UV:
			return std::make_unique<UV>();
		case GifRegisters::XYZ2:
			return std::make_unique<XYZ2>();
		case GifRegisters::TEX0:
			return std::make_unique<TEX0>();
		case GifRegisters::FOG:
			return std::make_unique<FOG>();
		case GifRegisters::FOGCOL:
			return std::make_unique<FOGCOL>();
		case GifRegisters::SCISSOR:
			return std::make_unique<SCISSOR>();
		case GifRegisters::SIGNAL:
			return std::make_unique<SIGNAL>();
		case GifRegisters::FINISH:
			return std::make_unique<FINISH>();
		case GifRegisters::LABEL:
			return std::make_unique<LABEL>();
		default:
			std::unreachable();
	}

	return nullptr;
}
