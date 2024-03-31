#include "machine.h"

#include <algorithm>
#include "registers.h"
#include "types.h"
#include "logger.h"

Machine machine;

auto Machine::TryStartBlock(const std::string& name) -> bool
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

		blocks.emplace_back(name);
		// Questionable usage of end and iterators here...
		currentBlockIt = --blocks.end();
		return true;
	}
}

auto Machine::TryStartMacro(const std::string& name) -> bool
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

auto Machine::TryEndBlockMacro() -> bool
{
	if(HasCurrentBlock())
	{
		FirstPassOptimize();
		backend->emit(*currentBlockIt);
		currentBlockIt = blocks.end();
		return true;
	}

	if(HasCurrentMacro())
	{
		currentMacroIt = macros.end();
		return true;
	}

	[[unlikely]] logger::error("No block to end\n");
	return false;
}

auto Machine::TryInsertMacro(const std::string& name) -> bool
{
	if(!HasCurrentBlockOrMacro()) [[unlikely]]
	{
		logger::error("No block or macro to insert macro into\n");
		return false;
	}

	const auto& macro = macros.find(name);
	if(macro != macros.end())
	{
		for(const auto& reg : macro->second.registers)
		{
			CurrentBlockMacro().registers.push_back(reg->Clone());
		}

		return true;
	}

	[[unlikely]] logger::error("Macro with name %s does not exist\n", name.c_str());
	return false;
}

auto Machine::TryInsertMacro(const std::string& name, Vec2 xyOffset) -> bool
{
	if(!HasCurrentBlockOrMacro()) [[unlikely]]
	{
		logger::error("No block or macro to insert macro into\n");
		return false;
	}

	const auto& macro = macros.find(name);
	if(macro != macros.end()) [[likely]]
	{
		GIFBlock tmpMacro = macro->second;
		for(const auto& reg : tmpMacro.registers)
		{
			if(reg->GetID() == GifRegisterID::PRIM)
			{
				// Copies the register
				XYZ2 xyz2 = dynamic_cast<XYZ2&>(*reg);
				if(!xyz2.value.has_value())
				{
					std::unreachable();
				}

				xyz2.value->x += xyOffset.x;
				xyz2.value->y += xyOffset.y;
				CurrentBlockMacro().registers.push_back(std::make_unique<XYZ2>(xyz2));
			}
			else
			{
				CurrentBlockMacro().registers.push_back(reg->Clone());
			}
		}
		return true;
	}

	[[unlikely]] logger::error("Macro with name %s does not exist\n", name.c_str());
	return false;
}

auto Machine::TrySetRegister(std::unique_ptr<GifRegister> reg) -> bool
{
	if(!HasCurrentBlockOrMacro())
	{
		logger::error("Not in current block");
	}
	else if(CurrentBlockMacro().HasRegister() && !CurrentBlockMacro().CurrentRegister().Ready())
	{
		logger::error("Current register is not fulfilled");
	}
	else [[likely]]
	{
		CurrentBlockMacro().registers.emplace_back(std::move(reg));
		return true;
	}
	return false;
}

auto Machine::TryPushReg(int32_t value) -> bool
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister()) [[likely]]
	{
		CurrentBlockMacro().CurrentRegister().Push(value);
		return true;
	}

	logger::error("There is no block, macro or register to push a integer to.");
	return false;
}

auto Machine::TryPushReg(Vec2 value) -> bool
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister()) [[likely]]
	{
		CurrentBlockMacro().CurrentRegister().Push(value);
		return true;
	}

	logger::error("There is no block, macro or register to push a Vec2 to.");
	return false;
}

auto Machine::TryPushReg(Vec3 value) -> bool
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister()) [[likely]]
	{
		CurrentBlockMacro().CurrentRegister().Push(value);
		return true;
	}

	logger::error("There is no block, macro or register to push a Vec3 to.");
	return false;
}

auto Machine::TryPushReg(Vec4 value) -> bool
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister()) [[likely]]
	{
		CurrentBlockMacro().CurrentRegister().Push(value);
		return true;
	}

	logger::error("There is no block, macro or register to push a Vec4 to.");
	return false;
}

auto Machine::TryApplyModifier(RegModifier mod) -> bool
{
	if(HasCurrentBlockOrMacro() && CurrentBlockMacro().HasRegister()) [[likely]]
	{
		return CurrentBlockMacro().CurrentRegister().ApplyModifier(mod);
	}

	logger::error("There is no block or register to apply a modifier to.");
	return false;
}

// First pass, doesn't know anything about the output format
// Second pass would be in the backend
void Machine::FirstPassOptimize()
{
	// Dead store Elimination
	if(OptimizeConfig[DEAD_STORE_ELIMINATION])
	{
		auto lastRegIt = CurrentBlock().registers.end();

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
			if(reg->GetID() == GifRegisterID::PRIM)
			{
				logger::info("Packing Prim into GIFTAG");
				CurrentBlock().prim = reg->Clone();
				break;
			}
		}
	}
}
