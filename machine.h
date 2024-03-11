#pragma once

#include <string>
#include <list>
#include <map>

#include "registers.h"
#include "backend/backend.hpp"

class Machine
{
	Backend *backend = nullptr;

	std::list<GIFBlock> blocks;
	std::map<std::string, GIFBlock> macros;
	// Do not use this to push registers
	GIFBlock *currentBlock;
	// Do not use this to push registers
	GIFBlock *currentMacro;
	// Use THIS to push registers
	GIFBlock *currentBlockMacro;

	GifRegister *currentRegister;

public:
	// this still leaks memory
	~Machine(){
		for(auto& block : blocks)
		{
			auto regs = std::unique(block.registers.begin(), block.registers.end());
			std::for_each(regs, block.registers.end(), [](GifRegister* reg) { delete reg; });
			block.registers.clear();
		}
		blocks.clear();
		macros.clear();
	}

	void SetBackend(Backend *backend) noexcept { this->backend = backend; };
	bool TryStartBlock(const std::string name);
	bool TryStartMacro(const std::string name);
	bool TryEndBlockMacro();
	bool TryInsertMacro(const std::string name);
	bool TryInsertMacro(const std::string name, Vec2 v);
	bool TrySetRegister(GifRegister *reg);
	bool TryPushReg(int32_t i);
	bool TryPushReg(Vec2 v);
	bool TryPushReg(Vec3 v3);
	bool TryPushReg(Vec4 v4);
	bool TryApplyModifier(RegModifier mod);
};

extern Machine machine;
