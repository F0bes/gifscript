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

class DummyBackend : public Backend
{
public:
	DummyBackend() = default;
	~DummyBackend() override = default;

	bool arg_parse(int argc, char** argv) override
	{
		return true;
	}

	void set_output(const std::string_view& output) override
	{
		(void)output;
	}

	void print_help() const override
	{
	}

	void emit(GIFBlock& block) override
	{
		(void)block;
	}
};

static DummyBackend dummy_backend;
