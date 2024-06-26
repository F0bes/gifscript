#pragma once

#include <string>
#include <list>
#include <map>
#include <bitset>
#include <utility>

#include "registers.hpp"
#include "backend.hpp"

class Machine
{
	Backend* backend = &dummy_backend;

	std::list<GIFBlock> blocks;
	std::map<std::string, GIFBlock> macros;
	// Do not use this to push registers
	std::list<GIFBlock>::iterator currentBlockIt = blocks.end();
	// Do not use this to push registers
	std::map<std::string, GIFBlock>::iterator currentMacroIt = macros.end();

	std::bitset<8> OptimizeConfig = std::bitset<8>().set();

	bool HasCurrentBlock() const noexcept { return currentBlockIt != blocks.end(); }
	bool HasCurrentMacro() const noexcept { return currentMacroIt != macros.end(); }
	bool HasCurrentBlockOrMacro() const noexcept { return HasCurrentBlock() || HasCurrentMacro(); }

	GIFBlock& CurrentBlock() noexcept { return *currentBlockIt; }
	GIFBlock& CurrentMacro() noexcept { return (*currentMacroIt).second; }
	// It is on the caller to ensure that either a block or macro is active
	// If neither are active, this is UB
	GIFBlock& CurrentBlockMacro()
	{
		if(HasCurrentBlock())
		{
			return *currentBlockIt;
		}
		else if(HasCurrentMacro())
		{
			return (*currentMacroIt).second;
		}
		logger::error("FATAL: PLEASE REPORT IF THIS HAPPENS\n");
		std::unreachable();
	}

	void EndBlockMacro() noexcept
	{
		currentBlockIt = blocks.end();
		currentMacroIt = macros.end();
	}

public:
	enum Optimization
	{
		DEAD_STORE_ELIMINATION = 1,
		USE_TAG_PRIM = 2,
	};

	~Machine()
	{
		blocks.clear();
		macros.clear();
	}

	void SetBackend(Backend* backend) noexcept { this->backend = backend; };
	bool TryStartBlock(const std::string&);
	bool TryStartMacro(const std::string&);
	bool TryEndBlockMacro();
	bool TryInsertMacro(const std::string&);
	bool TryInsertMacro(const std::string&, Vec2);
	bool TrySetRegister(std::unique_ptr<GifRegister> reg);
	bool TryPushReg(int32_t);
	bool TryPushReg(Vec2);
	bool TryPushReg(Vec3);
	bool TryPushReg(Vec4);
	bool TryApplyModifier(RegModifier);

	void DisableOptimization(Optimization op)
	{
		OptimizeConfig[op] = false;
	}

private:
	void FirstPassOptimize();
};

extern Machine machine;
