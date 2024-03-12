#include "machine.h"

#include <algorithm>
#include "registers.h"
#include "types.h"
#include "logger.h"

Machine machine;

bool Machine::TryStartBlock(const std::string name)
{
	if(currentBlock)
	{
		logger::error("Still waiting for you to end the block %s\n", currentBlock->name.c_str());
		return false;
	}
	else if (currentMacro)
	{
		logger::error("Still waiting for you to end the macro %s\n", currentMacro->name.c_str());
		return false;
	}
	else
	{
		auto block_name_dup = std::find_if(blocks.begin(), blocks.end(), [&name](GIFBlock& block) { return block.name == name; });\
		if(block_name_dup != blocks.end())
		{
			logger::error("Block with name %s already exists\n", name.c_str());
			return false;
		}

		auto macro_name_dup = macros.find(name);
		if(macro_name_dup != macros.end())
		{
			logger::error("Macro with name %s already exists\n", name.c_str());
			return false;
		}

		currentBlock = &blocks.emplace_back(GIFBlock(name));
		currentBlockMacro = currentBlock;
		return true;
	}
}

bool Machine::TryStartMacro(const std::string name)
{
	if(currentMacro)
	{
		logger::error("Still waiting for you to end the macro %s\n", currentMacro->name.c_str());
		return false;
	}
	else if (currentBlock)
	{
		logger::error("Still waiting for you to end the block %s\n", currentBlock->name.c_str());
		return false;
	}
	else
	{
		auto block_name_dup = std::find_if(blocks.begin(), blocks.end(), [&name](GIFBlock& block) { return block.name == name; });
		if(block_name_dup != blocks.end())
		{
			logger::error("Block with name %s already exists\n", name.c_str());
			return false;
		}
		
		auto macro_name_dup = macros.find(name);
		if(macro_name_dup != macros.end())
		{
			logger::error("Macro with name %s already exists\n", name.c_str());
			return false;
		}

		macros[name] = GIFBlock(name);
		currentMacro = &macros[name];
		currentBlockMacro = currentMacro;
		return true;
	}
}

bool Machine::TryEndBlockMacro()
{
	if(currentBlock)
	{
		backend->emit(currentBlock);
		currentBlock = nullptr;
		currentBlockMacro = nullptr;
		return true;
	}
	else if (currentMacro)
	{
		currentMacro = nullptr;
		currentBlockMacro = nullptr;
		return true;
	}
	else
	{
		logger::error("No block to end\n");
		return false;
	}
}

bool Machine::TryInsertMacro(const std::string name)
{
	if(currentMacro)
	{
		logger::error("Support for nested macros is not implemented yet!\n");
		return false;
	}
	else if (!currentBlock)
	{
		logger::error("No block to insert macro into\n");
		return false;
	}
	else
	{
		auto macro = macros.find(name);
		if(macro != macros.end())
		{
			currentBlock->registers.insert(currentBlock->registers.end(), macro->second.registers.begin(), macro->second.registers.end());
			return true;
		}
		else
		{
			logger::error("Macro with name %s does not exist\n", name.c_str());
			return false;
		}
	}
}

bool Machine::TryInsertMacro(const std::string name, Vec2 v)
{
	if(currentMacro)
	{
		logger::error("Support for nested macros is not implemented yet!\n");
		return false;
	}
	else if (!currentBlock)
	{
		logger::error("No block to insert macro into\n");
		return false;
	}
	else
	{
		auto macro = macros.find(name);
		if(macro != macros.end())
		{
			GIFBlock tmpMacro = GIFBlock(macro->second);

			for(auto reg : tmpMacro.registers)
			{
				if(reg->GetID() == 0x05)
				{
					XYZ2 xyz2 = dynamic_cast<XYZ2&>(*reg);
					
					auto tmpXYZ2 = std::make_shared<XYZ2>(xyz2);
					
					tmpXYZ2->value->i_x += v.i_x;
					tmpXYZ2->value->i_y += v.i_y;
					currentBlock->registers.push_back(tmpXYZ2);
				}
				else
					currentBlock->registers.push_back(reg);
			}
			return true;
		}
		else
		{
			logger::error("Macro with name %s does not exist\n", name.c_str());
			return false;
		}
	}
}
bool Machine::TrySetRegister(std::shared_ptr<GifRegister> reg)
{
	if(!currentBlockMacro)
	{
		logger::error("Not in current block");
	}
	else if(currentRegister && !currentRegister->Ready())
	{
		logger::error("Current register is not fulfilled");
	}
	else
	{
		currentRegister = currentBlockMacro->registers.emplace_back(reg);
		return true;
	}
	return false;
}

bool Machine::TryPushReg(int32_t i)
{
	if(currentBlockMacro && currentRegister)
	{
		currentRegister->Push(i);
		return true;
	}
	else
	{
		logger::error("There is no block, macro or register to push a integer to.");
		return false;
	}
}

bool Machine::TryPushReg(Vec2 v)
{
	if(currentBlockMacro && currentRegister)
	{
		currentRegister->Push(v);
		return true;
	}
	else
	{
		logger::error("There is no block, macro or register to push a Vec2 to.");
		return false;
	}
}

bool Machine::TryPushReg(Vec3 v3)
{
	if(currentBlockMacro && currentRegister)
	{
		currentRegister->Push(v3);
		return true;
	}
	else
	{
		logger::error("There is no block, macro or register to push a Vec3 to.");
		return false;
	}
}

bool Machine::TryPushReg(Vec4 v4)
{
	if(currentBlockMacro && currentRegister)
	{
		currentRegister->Push(v4);
		return true;
	}
	else
	{
		logger::error("There is no block, macro or register to push a Vec4 to.");
		return false;
	}
}

bool Machine::TryApplyModifier(RegModifier mod)
{
	if(currentBlockMacro && currentRegister)
	{
		return currentRegister->ApplyModifier(mod);
	}
	else
	{
		logger::error("There is no block or register to apply a modifier to.");
		return false;
	}
}
