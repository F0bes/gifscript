#pragma once
#include "registers.h"

class Backend
{
	public:
	Backend() = default;
	virtual ~Backend() = default;

	virtual bool arg_parse(int argc, char** argv) = 0;

	virtual void print_help() const = 0;

	virtual void emit(GIFBlock* block) = 0;
};
