#include "registers.h"

std::shared_ptr<GifRegister> GenReg(GifRegisters reg)
{
	switch (reg)
	{
		case GifRegisters::PRIM:
			return std::make_shared<PRIM>();
		case GifRegisters::RGBAQ:
			return std::make_shared<RGBAQ>();
		case GifRegisters::XYZ2:
			return std::make_shared<XYZ2>();
		case GifRegisters::FOG:
			return std::make_shared<FOG>();
		case GifRegisters::FOGCOL:
			return std::make_shared<FOGCOL>();
		case GifRegisters::SCISSOR:
			return std::make_shared<SCISSOR>();
		case GifRegisters::SIGNAL:
			return std::make_shared<SIGNAL>();
		case GifRegisters::FINISH:
			return std::make_shared<FINISH>();
		case GifRegisters::LABEL:
			return std::make_shared<LABEL>();
	}

	return nullptr;
}
