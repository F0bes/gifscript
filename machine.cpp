#include "machine.h"

#include <algorithm>
#include "registers.h"
#include "types.h"
#include "logger.h"

Machine machine;

bool Machine::TryStartBlock(const std::string name)
{
	if(HasCurrentBlock()) [[unlikely]]
	{
		logger::error("Still waiting for you to end the block %s\n", CurrentBlock().name.c_str());
		return false;
	}
	else if(HasCurrentMacro()) [[unlikely]]
	{
		logger::error("Still waiting for you to end the macro %s\n", CurrentMacro().name.c_str());
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

		blocks.push_back(GIFBlock(name));
		// Questionable usage of end and iterators here...
		currentBlockIt = --blocks.end();
		return true;
	}
}

bool Machine::TryStartMacro(const std::string name)
{
	if(HasCurrentMacro()) [[unlikely]]
	{
		logger::error("Still waiting for you to end the macro %s\n", currentMacroIt->second.name.c_str());
		return false;
	}
	else if(HasCurrentBlock()) [[unlikely]]
	{
		logger::error("Still waiting for you to end the block %s\n", currentBlockIt->name.c_str());
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

		macros.emplace(name, GIFBlock(name));
		currentMacroIt = macros.emplace(name, GIFBlock(name)).first;
		return true;
	}
}

bool Machine::TryEndBlockMacro()
{
	if(HasCurrentBlock())
	{
		FirstPassOptimize();
		backend->emit(*currentBlockIt);
		currentBlockIt = blocks.end();
		return true;
	}
	else if(HasCurrentMacro())
	{
		currentMacroIt = macros.end();
		return true;
	}
	else [[unlikely]]
	{
		logger::error("No block to end\n");
		return false;
	}
}

bool Machine::TryInsertMacro(const std::string name)
{
	if(!HasCurrentBlockOrMacro()) [[unlikely]]
	{
		logger::error("No block or macro to insert macro into\n");
		return false;
	}
	else
	{
		const auto& macro = macros.find(name);
		if(macro != macros.end())
		{
			for(const auto& reg : macro->second.registers)
			{
				CurrentBlockMacro().registers.push_back(reg->Clone());
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

bool Machine::TryInsertMacro(const std::string name, Vec2 v)
{
	if(!HasCurrentBlockOrMacro()) [[unlikely]]
	{
		logger::error("No block or macro to insert macro into\n");
		return false;
	}
	else
	{
		const auto& macro = macros.find(name);
		if(macro != macros.end())
		{
			GIFBlock tmpMacro = macro->second;

			for(const auto& reg : tmpMacro.registers)
			{
				if(reg->GetID() == 0x05)
				{
					// Copies the register
					XYZ2 xyz2 = dynamic_cast<XYZ2&>(*reg);

					xyz2.value->x += v.x;
					xyz2.value->y += v.y;
					CurrentBlockMacro().registers.push_back(std::make_unique<XYZ2>(xyz2));
				}
				else
				{
					CurrentBlockMacro().registers.push_back(reg->Clone());
				}
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

bool Machine::TrySetRegister(std::unique_ptr<GifRegister> reg)
{
	if(!HasCurrentBlockOrMacro()) [[unlikely]]
	{
		logger::error("Not in current block");
	}
	else if(CurrentBlockMacro().HasRegister() && !CurrentBlockMacro().CurrentRegister().Ready()) [[unlikely]]
	{
		logger::error("Current register is not fulfilled");
	}
	else
	{
		CurrentBlockMacro().registers.emplace_back(std::move(reg));
		return true;
	}
	return false;
}

bool Machine::TryPushReg(int32_t i)
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister())
	{
		CurrentBlockMacro().CurrentRegister().Push(i);
		return true;
	}
	else [[unlikely]]
	{
		logger::error("There is no block, macro or register to push a integer to.");
		return false;
	}
}

bool Machine::TryPushReg(Vec2 v2)
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister())
	{
		CurrentBlockMacro().CurrentRegister().Push(v2);
		return true;
	}
	else [[unlikely]]
	{
		logger::error("There is no block, macro or register to push a Vec2 to.");
		return false;
	}
}

bool Machine::TryPushReg(Vec3 v3)
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister())
	{
		CurrentBlockMacro().CurrentRegister().Push(v3);
		return true;
	}
	else [[unlikely]]
	{
		logger::error("There is no block, macro or register to push a Vec3 to.");
		return false;
	}
}

bool Machine::TryPushReg(Vec4 v4)
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister())
	{
		CurrentBlockMacro().CurrentRegister().Push(v4);
		return true;
	}
	else [[unlikely]]
	{
		logger::error("There is no block, macro or register to push a Vec4 to.");
		return false;
	}
}

bool Machine::TryApplyModifier(RegModifier mod)
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister())
	{
		return CurrentBlockMacro().CurrentRegister().ApplyModifier(mod);
	}
	else [[unlikely]]
	{
		logger::error("There is no block or register to apply a modifier to.");
		return false;
	}
}

// First pass, doesn't know anything about the output format
// Second pass would be in the backend
void Machine::FirstPassOptimize()
{
	// Dead store Elimination
	if(OptimizeConfig[DEAD_STORE_ELIMINATION])
	{
		std::list<std::unique_ptr<GifRegister>>::iterator lastRegIt = CurrentBlock().registers.end();

		for(auto regIt = CurrentBlock().registers.begin(); regIt != CurrentBlock().registers.end(); regIt++)
		{
			if(regIt->operator->()->HasSideEffects())
			{
				lastRegIt = CurrentBlock().registers.end();
			}
			else if(lastRegIt != CurrentBlock().registers.end())
			{
				if(lastRegIt->operator->()->GetID() == regIt->operator->()->GetID())
				{
					logger::info("Dead store elimination: %s", lastRegIt->operator->()->GetName().cbegin());
					CurrentBlock().registers.remove(*lastRegIt);
					// Set lastReg to this register iterator
					lastRegIt = regIt;
				}
				else
				{
					lastRegIt = regIt;
				}
			}
			else
			{
				lastRegIt = regIt;
			}
		}
	}

	// Packing Prim into GIFTAG (should have no side effects)
	if(OptimizeConfig[USE_TAG_PRIM])
	{
		for(const auto& reg : CurrentBlock().registers)
		{
			if(reg->GetID() == 0)
			{
				logger::info("Packing Prim into GIFTAG");
				CurrentBlock().prim = reg->Clone();
				break;
			}
		}
	}
}
