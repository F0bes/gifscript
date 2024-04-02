#include <fmt/format.h>

#include "logger.hpp"
#include "registers.hpp"
#include "machine.hpp"
#include "backend.hpp"
#include "c_code.hpp"
#include "version.hpp"
#include "parser.h"
#include "parser.c"

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

void ParsePRIM(const uint32_t& prim)
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

void ParseRGBAQ(const uint32_t& rgbaq)
{
	Parse(lparser, REG, new std::any(GifRegisters::RGBAQ), &valid);
	Vec4 color = Vec4((rgbaq >> 24) & 0xFF, (rgbaq >> 16) & 0xFF, (rgbaq >> 8) & 0xFF, rgbaq & 0xFF);
	Parse(lparser, VEC4, new std::any(color), &valid);
	Parse(lparser, 0, 0, &valid);
}

void ParseUV(const uint32_t& uv_reg)
{
	Parse(lparser, REG, new std::any(GifRegisters::UV), &valid);
	Vec2 uv_vec = Vec2((uv_reg >> 16) & 0xFFFF, uv_reg & 0xFFFF);
	Parse(lparser, VEC2, new std::any(uv_vec), &valid);
	Parse(lparser, 0, nullptr, &valid);
}

void ParseXYZ2(const uint32_t& xyz2)
{
	Parse(lparser, REG, new std::any(GifRegisters::XYZ2), &valid);
	Vec3 xyz = Vec3((xyz2 >> 20) & 0xFFFF, (xyz2 >> 12) & 0xFF, xyz2 & 0xFF);
	Parse(lparser, VEC3, new std::any(xyz), &valid);
	Parse(lparser, 0, 0, &valid);
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
					uint32_t gifreg = gifregs >> (j * 4) & 0xF;
					logger::warn("GIFREG is %x, %016X", gifreg, *ptr);

					switch(gifreg)
					{
						case 0x0E:
						{
							uint32_t data = *ptr;
							ptr++;
							uint32_t dest = *ptr;
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
			   "    Prints the version of gifscript\n\t"
			   "  --oneshot-parse\n\t"
			   "    Parses the entire file in one go. Probably faster, but you lose proper parsing error handling\n\t"
			   "Optimization settings:\n\t"
			   "  --keep-deadstore\n\t"
			   "    Disables dead store optimization. (Consecutive writes to stateless registers)\n\t"
			   " --no-tag-prim\n\t"
			   "    Disables packing the first PRIM write into the GIFTag\n\t"
			   "Valid backends are:\n\t"
			   "  c_code(default)\n\t"
			   "    Generates a c file with an array for each gif block\n"
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
			fmt::print("Gifscript version: {}\n", GIT_VERSION);
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
		logger::info("No backend specified, using default 'c_code'");
		backend = new c_code_backend();
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
	delete backend;
	fclose(fin);
	return 0;
}
