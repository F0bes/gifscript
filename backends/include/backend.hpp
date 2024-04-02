#pragma once
#include "registers.hpp"

class Backend
{
public:
	Backend() = default;
	virtual ~Backend() = default;

	virtual bool arg_parse(int argc, char** argv) = 0;

	virtual void set_output(const std::string_view& output) = 0;

	virtual void print_help() const = 0;

	virtual void emit(GIFBlock& block) = 0;
};
