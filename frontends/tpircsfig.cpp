#include <fmt/format.h>

#include "logger.hpp"
#include "registers.hpp"
#include "machine.hpp"
#include "backend.hpp"
#include "c_code.hpp"
#include "gifscript_backend.hpp"
#include "version.hpp"
#include "parser.h"
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "parser.c"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

static bool valid = true;
static Backend* backend = nullptr;
static void* lparser;

struct GIFTag
{
	uint32_t NLOOP : 15;
	uint32_t EOP : 1;
	uint32_t : 16;
	uint32_t : 14;
	uint32_t PRE : 1;
	uint32_t PRIM : 11;
	uint32_t FLG : 2;
	uint32_t NREG : 4;
	uint64_t REGS;
};

void ParsePRIM(const uint64_t& prim)
{
	Parse(lparser, REG, new std::any(GifRegisters::PRIM), &valid);
	switch(prim & 0x7)
	{
		case 0:
			Parse(lparser, MOD, new std::any(RegModifier::Point), &valid);
			break;
		case 1:
			Parse(lparser, MOD, new std::any(RegModifier::Line), &valid);
			break;
		case 2:
			Parse(lparser, MOD, new std::any(RegModifier::TriangleStrip), &valid);
			break;
		case 3:
			Parse(lparser, MOD, new std::any(RegModifier::Triangle), &valid);
			break;
		case 4:
			Parse(lparser, MOD, new std::any(RegModifier::TriangleStrip), &valid);
			break;
		case 5:
			Parse(lparser, MOD, new std::any(RegModifier::TriangleFan), &valid);
			break;
		case 6:
			Parse(lparser, MOD, new std::any(RegModifier::Sprite), &valid);
			break;
		case 7:
			logger::error("Invalid PRIM type: 7");
			break;
		default:
			std::unreachable();
	}

	if(prim & 0x8)
	{
		Parse(lparser, MOD, new std::any(RegModifier::Gouraud), &valid);
	}

	if(prim & 0x10)
	{
		Parse(lparser, MOD, new std::any(RegModifier::Texture), &valid);
	}

	if(prim & 0x20)
	{
		Parse(lparser, MOD, new std::any(RegModifier::Fogging), &valid);
	}

	Parse(lparser, 0, nullptr, &valid);
}

void ParseRGBAQ(const uint64_t& rgbaq)
{
	Parse(lparser, REG, new std::any(GifRegisters::RGBAQ), &valid);
	Vec4 color = Vec4(rgbaq & 0xFF, (rgbaq >> 8) & 0xFF, (rgbaq >> 16) & 0xFF, (rgbaq >> 24) & 0xFF);
	Parse(lparser, VEC4, new std::any(color), &valid);
	Parse(lparser, 0, 0, &valid);
}

void ParseUV(const uint64_t& uv_reg)
{
	Parse(lparser, REG, new std::any(GifRegisters::UV), &valid);
	Vec2 uv_vec = Vec2((uv_reg >> 4) & 0x3FFF, (uv_reg >> 20) & 0x3FFF);
	Parse(lparser, VEC2, new std::any(uv_vec), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseXYZ2(const uint64_t& xyz2)
{
	Parse(lparser, REG, new std::any(GifRegisters::XYZ2), &valid);
	Vec3 xyz = Vec3((xyz2 >> 4) & 0xFFFF, (xyz2 >> 20) & 0xFFFF, (xyz2 >> 36) & 0xFFFF);
	Parse(lparser, VEC3, new std::any(xyz), &valid);
	Parse(lparser, 0, 0, &valid);
}

void ParseTEX0(const uint64_t& tex0)
{
	Parse(lparser, REG, new std::any(GifRegisters::TEX0), &valid);
	Parse(lparser, NUMBER_LITERAL, new std::any(static_cast<uint32_t>(tex0 & 0x3FFF)), &valid);
	Parse(lparser, NUMBER_LITERAL, new std::any(static_cast<uint32_t>((tex0 >> 14) & 0x3F)), &valid);
	Parse(lparser, VEC2, new std::any(Vec2((tex0 >> 26) & 0xF, (tex0 >> 30) & 0xF)), &valid);
	switch(tex0 >> 20 & 0x3F)
	{
		case 0:
			Parse(lparser, MOD, new std::any(RegModifier::CT32), &valid);
			break;
		case 1:
			Parse(lparser, MOD, new std::any(RegModifier::CT24), &valid);
			break;
		case 2:
			Parse(lparser, MOD, new std::any(RegModifier::CT16), &valid);
			break;
		default:
			logger::error("Invalid TBP value: {}", tex0 >> 20 & 0xF);
	}

	switch((tex0 >> 35) & 0x3)
	{
		case 0:
			Parse(lparser, MOD, new std::any(RegModifier::Modulate), &valid);
			break;
		case 1:
			Parse(lparser, MOD, new std::any(RegModifier::Decal), &valid);
			break;
		case 2:
			Parse(lparser, MOD, new std::any(RegModifier::Highlight), &valid);
			break;
		case 3:
			Parse(lparser, MOD, new std::any(RegModifier::Highlight2), &valid);
			break;
	}

	Parse(lparser, 0, nullptr, &valid);
}

void ParseFOG(const uint64_t& fog)
{
	Parse(lparser, REG, new std::any(GifRegisters::FOG), &valid);
	Parse(lparser, NUMBER_LITERAL, new std::any(static_cast<uint32_t>((fog >> 56) & 0xFF)), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseFOGCOL(const uint64_t& fogcol)
{
	Parse(lparser, REG, new std::any(GifRegisters::FOGCOL), &valid);
	Vec3 color = Vec3(fogcol & 0xFF, (fogcol >> 8) & 0xFF, (fogcol >> 16) & 0xFF);
	Parse(lparser, VEC3, new std::any(color), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseSCISSOR(const uint64_t& scissor)
{
	Parse(lparser, REG, new std::any(GifRegisters::SCISSOR), &valid);
	Vec4 scissor_vec = Vec4(scissor & 0x7FF, (scissor >> 16) & 0x7FF, (scissor >> 32) & 0x7FF, (scissor >> 48) & 0x7FF);
	Parse(lparser, VEC4, new std::any(scissor_vec), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseSIGNAL(const uint64_t& signal)
{
	Parse(lparser, REG, new std::any(GifRegisters::SIGNAL), &valid);
	Parse(lparser, VEC2, new std::any(Vec2(signal & UINT32_MAX, signal >> 32)), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseFINISH(const uint64_t& finish)
{
	Parse(lparser, REG, new std::any(GifRegisters::FINISH), &valid);
	Parse(lparser, NUMBER_LITERAL, new std::any(static_cast<uint32_t>(finish)), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseLABEL(const uint64_t& label)
{
	Parse(lparser, REG, new std::any(GifRegisters::LABEL), &valid);
	Parse(lparser, NUMBER_LITERAL, new std::any(static_cast<uint32_t>(label)), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void Scan(uint64_t* buffer, size_t size)
{
	uint64_t* ptr = buffer;

	// GIFTag
	GIFTag& tag = *reinterpret_cast<GIFTag*>(ptr);
	Parse(lparser, IDENTIFIER, new std::any(fmt::format("block_{:x}", reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(buffer))), &valid);
	Parse(lparser, BLOCK_START, nullptr, &valid);
	Parse(lparser, 0, nullptr, &valid);

	if(tag.PRE)
	{
		ParsePRIM(tag.PRIM);
	}

	ptr++;
	uint64_t gifregs = *ptr;
	ptr++;

	switch(tag.FLG)
	{
		case 0:
			for(int i = 0; i < tag.NLOOP; i++)
			{
				for(int j = 0; j < tag.NREG; j++)
				{
					uint64_t gifreg = gifregs >> (j * 4) & 0xF;
					logger::warn("GIFREG is %x, %016X", gifreg, *ptr);

					switch(gifreg)
					{
						case 0x0E:
						{
							uint64_t data = *ptr;
							ptr++;
							uint64_t dest = *ptr;
							ptr++;
							switch(static_cast<GifRegisterID>(dest))
							{
								case GifRegisterID::PRIM:
									ParsePRIM(data);
									break;
								case GifRegisterID::RGBAQ:
									ParseRGBAQ(data);
									break;
								case GifRegisterID::UV:
									ParseUV(data);
									break;
								case GifRegisterID::XYZ2:
									ParseXYZ2(data);
									break;
								case GifRegisterID::TEX0:
									ParseTEX0(data);
									break;
								case GifRegisterID::FOG:
									ParseFOG(data);
									break;
								case GifRegisterID::FOGCOL:
									ParseFOGCOL(data);
									break;
								case GifRegisterID::SCISSOR:
									ParseSCISSOR(data);
									break;
								case GifRegisterID::SIGNAL:
									ParseSIGNAL(data);
									break;
								case GifRegisterID::FINISH:
									ParseFINISH(data);
									break;
								case GifRegisterID::LABEL:
									ParseLABEL(data);
									break;
								default:
									logger::error("Unsupported gs register");
							}
						}
						break;
						default:
							logger::error("Only supports AD gifreg currently");
					}
				}
			}
			break;
		default:
			logger::error("Unsupported FLG: %u", (uint32_t)tag.FLG);
	}
	Parse(lparser, BLOCK_END, nullptr, &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void print_help(char* argv0)
{
	fmt::print("Usage: {} <file> <output> [--backend=<backend>] [--b<backend arguments>]\n\t"
			   "General Arguments:\n\t"
			   "  --help, -h\n\t"
			   "    Prints this help message\n\t"
			   "  --version, -v\n\t"
			   "    Prints the version of tpircsfig\n\t"
			   "Valid backends are:\n\t"
			   "  c_code(default)\n\t"
			   "    Generates a c file with an array for each gif block\n"
			   "  gifscript\n\t"
			   "    Generates a gifscript file. Mostly used for debugging or tpircsfig\n"
			   "For backend specific help, please pass --bhelp to your backend\n",
		argv0);
};


std::string file_in;
std::string file_out;
auto main(int argc, char** argv) -> int
{
	machine.DisableOptimization(Machine::Optimization::DEAD_STORE_ELIMINATION);
	//machine.DisableOptimization(Machine::Optimization::USE_TAG_PRIM);

	if(argc < 2)
	{
		print_help(argv[0]);
		return 1;
	}
	for(int i = 1; i < argc; i++)
	{
		std::string_view arg = argv[i];
		if(arg.starts_with("--backend="))
		{
			std::string_view backend_str = arg.substr(strlen("--backend="));
			if(backend_str == "c_code")
			{
				fmt::print("Using C backend\n");
				backend = new c_code_backend();
				if(!backend->arg_parse(argc, argv))
				{
					fmt::print("Use --bhelp for valid backend configuration arguments\n");
					return 1;
				}
			}
			else if(backend_str == "gifscript")
			{
				fmt::print("Using gifscript backend\n");
				backend = new gifscript_backend();
				if(!backend->arg_parse(argc, argv))
				{
					fmt::print("Use --bhelp for valid backend configuration arguments\n");
					return 1;
				}
			}
			else
			{
				fmt::print("Unknown backend: {}\n", backend_str);
				return 1;
			}
		}
		else if(arg == "--help" || arg == "-h")
		{
			print_help(argv[0]);
			return 1;
		}
		else if(arg == "--version" || arg == "-v")
		{
			fmt::print("Gifscript(tpircsfig) version: {}\n", GIT_VERSION);
			return 0;
		}
		else if(arg == "--keep-deadstore")
		{
			machine.DisableOptimization(Machine::Optimization::DEAD_STORE_ELIMINATION);
		}
		else if(arg == "--no-tag-prim")
		{
			machine.DisableOptimization(Machine::Optimization::USE_TAG_PRIM);
		}
		else if(file_in.empty() && !arg.starts_with("-"))
		{
			file_in = arg;
		}
		else if(file_out.empty() && !arg.starts_with("-"))
		{
			file_out = arg;
		}
		else if(!arg.starts_with("--b"))
		{
			fmt::print("Unknown argument: {}\n", arg);
			return 1;
		}
	}

	if(backend == nullptr)
	{
		logger::info("No backend specified, using default 'gifscript'");
		backend = new gifscript_backend();
		if(!backend->arg_parse(argc, argv))
		{
			fmt::print("Use --bhelp for valid backend configuration arguments\n");
			return 1;
		}
	}

	machine.SetBackend(backend);

	backend->set_output(file_out);

	if(file_in.empty())
	{
		fmt::print("No input file specified\n");
		return 1;
	}

	if(file_out.empty())
	{
		fmt::print("No output file specified. Printing to stdout\n");
	}

	lparser = ParseAlloc(malloc);
	const size_t buffer_size = 8192;
	std::array<uint64_t, buffer_size> buffer;
	FILE* fin;
	unsigned long numbytes;

	fin = fopen(file_in.c_str(), "rb");
	if(fin == nullptr)
	{
		fmt::print("Failed to open file: {}\n", file_in);
		return 1;
	}
	fseek(fin, 0, SEEK_END);
	numbytes = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	if(numbytes > sizeof(buffer))
	{
		fmt::print("File {} bytes is too large for internal buffer {} bytes :(\n", numbytes, sizeof(buffer));
		return 1;
	}

	fread(buffer.data(), sizeof(uint64_t), numbytes, fin);

	Scan(buffer.data(), numbytes);

	ParseFree(lparser, free);
	delete backend;
	fclose(fin);
	return 0;
}
