#pragma once

#include "backend.hpp"

#include <functional>
#include <unordered_map>

class gifscript_backend : public Backend
{

public:
	gifscript_backend() = default;
	~gifscript_backend();

	bool arg_parse(int argc, char** argv) override;

	void set_output(const std::string_view& output) override
	{
		this->output = output;
	};

	void print_help() const override;

	void emit(GIFBlock& block) override;

	// Primitive dispatching
	static std::string emit_primitive(gifscript_backend*, const GifRegister&);
	static std::string emit_rgbaq(gifscript_backend*, const GifRegister&);
	static std::string emit_uv(gifscript_backend*, const GifRegister&);
	static std::string emit_xyz2(gifscript_backend*, const GifRegister&);
	static std::string emit_tex0(gifscript_backend*, const GifRegister&);
	static std::string emit_fog(gifscript_backend*, const GifRegister&);
	static std::string emit_fogcol(gifscript_backend*, const GifRegister&);
	static std::string emit_scissor(gifscript_backend*, const GifRegister&);
	static std::string emit_signal(gifscript_backend*, const GifRegister&);
	static std::string emit_finish(gifscript_backend*, const GifRegister&);
	static std::string emit_label(gifscript_backend*, const GifRegister&);

	std::unordered_map<uint32_t, std::function<std::string(gifscript_backend*, const GifRegister&)>> dispatch_table =
		{
			{0x00, gifscript_backend::emit_primitive},
			{0x01, gifscript_backend::emit_rgbaq},
			{0x03, gifscript_backend::emit_uv},
			{0x05, gifscript_backend::emit_xyz2},
			{0x06, gifscript_backend::emit_tex0},
			{0x0A, gifscript_backend::emit_fog},
			{0x3D, gifscript_backend::emit_fogcol},
			{0x40, gifscript_backend::emit_scissor},
			{0x60, gifscript_backend::emit_signal},
			{0x61, gifscript_backend::emit_finish},
			{0x62, gifscript_backend::emit_label}};

private:
	std::string output = "";
	FILE* file = nullptr;
	bool first_emit = true;
};
