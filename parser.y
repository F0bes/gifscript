%include {
#include <any>
#include <iostream>
#include <cassert>
#include "types.h"
#include "registers.h"
#include "parser.h"
#include "machine.h"

#include "backend/backend.hpp"
}

%syntax_error {
  std::cout << "Syntax error." << std::endl;
  // print the bad token
  *valid = false;
}

%token_type {std::any *}
%token_destructor { delete $$; }

//%destructor IDENTIFIER { delete $$; }
%extra_argument { bool* valid }

program ::= create_block. 
program ::= create_macro.
program ::= set_register.
program ::= end_block.
program ::= insert_macro.

// Register stuff

set_register ::= REG(A). {
	*valid = false;

	delete A;
}

set_register ::= REG(A) VEC4(B). {
	Vec4 val = std::any_cast<Vec4>(*B);
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A))) || !machine.TryPushReg(val)) {
		*valid = false;
	}

	delete A;
	delete B;
}

set_register ::= REG(A) VEC3(B). {
	Vec3 val = std::any_cast<Vec3>(*B);
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A))) || !machine.TryPushReg(val)) {
		*valid = false;
	}

	delete A;
	delete B;
}

set_register ::= REG(A) VEC2(B). {
	Vec2 val = std::any_cast<Vec2>(*B);
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A))) || !machine.TryPushReg(val)) {
		*valid = false;
	}

	delete A;
	delete B;
}

set_register ::= REG(A) MOD(B). {
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A)))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*B))) {
		*valid = false;
	}

	delete A;
	delete B;
}

set_register ::= REG(A) MOD(B) MOD(C). {
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A)))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*B))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*C))) {
		*valid = false;
	}

	delete A;
	delete B;
	delete C;
}

set_register ::= REG(A) MOD(B) MOD(C) MOD(D). {
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A)))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*B))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*C))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*D))) {
		*valid = false;
	}

	delete A;
	delete B;
	delete C;
	delete D;
}

set_register ::= REG(A) MOD(B) MOD(C) MOD(D) MOD(E). {
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A)))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*B))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*C))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*D))
	|| !machine.TryApplyModifier(std::any_cast<RegModifier>(*E))) {
		*valid = false;
	}

	delete A;
	delete B;
	delete C;
	delete D;
	delete E; 
}

set_register ::= REG(A) NUMBER_LITERAL(B). {
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A))) || !machine.TryPushReg(std::any_cast<uint32_t>(*B))) {
		*valid = false;
	}

	delete A;
	delete B;
}

// Block madness

create_block ::= IDENTIFIER(A) BLOCK_START. {
	*valid = machine.TryStartBlock(std::any_cast<std::string>(*A));

	delete A;
}

create_macro ::= MACRO IDENTIFIER(A) BLOCK_START. {
	*valid = machine.TryStartMacro(std::any_cast<std::string>(*A));

	delete A;
}


insert_macro ::= MACRO IDENTIFIER(A). {
	*valid = machine.TryInsertMacro(std::any_cast<std::string>(*A));

	delete A;
}

insert_macro ::= MACRO IDENTIFIER(A) VEC2(B). {
	*valid = machine.TryInsertMacro(std::any_cast<std::string>(*A), std::any_cast<Vec2>(*B));

	delete A;
	delete B;
}

end_block ::= BLOCK_END. {
	*valid = machine.TryEndBlockMacro();
}
