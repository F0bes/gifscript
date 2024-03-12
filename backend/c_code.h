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

	void emit(GIFBlock* block) override;

	// Primitive dispatching
	static std::string emit_primitive(c_code_backend*, std::shared_ptr<GifRegister> reg);
	static std::string emit_rgbaq(c_code_backend*,std::shared_ptr<GifRegister> reg);
	static std::string emit_xyz2(c_code_backend*,std::shared_ptr<GifRegister> reg);

	std::unordered_map<uint32_t, std::function<std::string(c_code_backend*, std::shared_ptr<GifRegister>)>> dispatch_table = 
	{
		{0, c_code_backend::emit_primitive},
		{1, c_code_backend::emit_rgbaq},
		{5, c_code_backend::emit_xyz2}
	};

	private:
	EmitMode emit_mode = EmitMode::USE_DEFS;
	std::string_view output = "";
	FILE* file = nullptr;
	bool first_emit = true;
};
