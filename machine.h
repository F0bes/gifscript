#pragma once

#include <string>
#include <list>
#include <map>
#include <bitset>

#include "registers.h"
#include "backend/backend.hpp"

class Machine
{
	Backend* backend = nullptr;

	std::list<GIFBlock> blocks;
	std::map<std::string, GIFBlock> macros;
	// Do not use this to push registers
	GIFBlock* currentBlock;
	// Do not use this to push registers
	GIFBlock* currentMacro;
	// Use THIS to push registers
	GIFBlock* currentBlockMacro;

	std::shared_ptr<GifRegister> currentRegister;

	std::bitset<8> OptimizeConfig = std::bitset<8>().set();

public:
	enum Optimization
	{
		DEAD_STORE_ELIMINATION = 1
	};

	~Machine()
	{
		blocks.clear();
		macros.clear();
	}

	void SetBackend(Backend* backend) noexcept { this->backend = backend; };
	bool TryStartBlock(const std::string name);
	bool TryStartMacro(const std::string name);
	bool TryEndBlockMacro();
	bool TryInsertMacro(const std::string name);
	bool TryInsertMacro(const std::string name, Vec2 v);
	bool TrySetRegister(std::shared_ptr<GifRegister> reg);
	bool TryPushReg(int32_t i);
	bool TryPushReg(Vec2 v);
	bool TryPushReg(Vec3 v3);
	bool TryPushReg(Vec4 v4);
	bool TryApplyModifier(RegModifier mod);

	void DisableOptimization(Optimization op)
	{
		OptimizeConfig[op] = false;
	}

private:
	void FirstPassOptimize();
};

extern Machine machine;
