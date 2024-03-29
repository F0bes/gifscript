#pragma once

#include "backend.hpp"

#include <functional>
#include <unordered_map>

class c_code_backend : public Backend
{
	enum class EmitMode
	{
		// Use the definitions found in
		// gs_gp.h
		// gif_tags.h
		USE_DEFS,
		// Do not use definitions and emit using raw values
		// Still requires gif_tags.h for data packing
		USE_MAGIC
	};

public:
	c_code_backend() noexcept;
	~c_code_backend();

	bool arg_parse(int argc, char** argv) override;

	void set_output(const std::string_view& output) override
	{
		this->output = output;
	};

	void print_help() const override;

	void emit(GIFBlock& block) override;

	// Primitive dispatching
	static std::string emit_primitive(c_code_backend*, const GifRegister&);
	static std::string emit_rgbaq(c_code_backend*, const GifRegister&);
	static std::string emit_uv(c_code_backend*, const GifRegister&);
	static std::string emit_xyz2(c_code_backend*, const GifRegister&);
	static std::string emit_tex0(c_code_backend*, const GifRegister&);
	static std::string emit_fog(c_code_backend*, const GifRegister&);
	static std::string emit_fogcol(c_code_backend*, const GifRegister&);
	static std::string emit_scissor(c_code_backend*, const GifRegister&);
	static std::string emit_signal(c_code_backend*, const GifRegister&);
	static std::string emit_finish(c_code_backend*, const GifRegister&);
	static std::string emit_label(c_code_backend*, const GifRegister&);

	std::unordered_map<uint32_t, std::function<std::string(c_code_backend*, const GifRegister&)>> dispatch_table =
		{
			{0x00, c_code_backend::emit_primitive},
			{0x01, c_code_backend::emit_rgbaq},
			{0x03, c_code_backend::emit_uv},
			{0x05, c_code_backend::emit_xyz2},
			{0x06, c_code_backend::emit_tex0},
			{0x0A, c_code_backend::emit_fog},
			{0x3D, c_code_backend::emit_fogcol},
			{0x40, c_code_backend::emit_scissor},
			{0x60, c_code_backend::emit_signal},
			{0x61, c_code_backend::emit_finish},
			{0x62, c_code_backend::emit_label}};

private:
	EmitMode emit_mode = EmitMode::USE_DEFS;
	std::string output = "";
	FILE* file = nullptr;
	bool first_emit = true;
};
