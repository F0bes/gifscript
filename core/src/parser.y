%include {
#include <any>
#include <iostream>
#include <cassert>
#include "types.hpp"
#include "registers.hpp"
#include "machine.hpp"
#include "parser.h"


#include "backend.hpp"
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
program ::= set_register params.
program ::= end_block.
program ::= insert_macro.

// Register stuff
params ::= param.
params ::= params param.

param ::= VEC4(A). {
	Vec4 val = std::any_cast<Vec4>(*A);
	if(!machine.TryPushReg(val)) {
		*valid = false;
	}

	delete A;
}

param ::= VEC3(A). {
	Vec3 val = std::any_cast<Vec3>(*A);
	if(!machine.TryPushReg(val)) {
		*valid = false;
	}

	delete A;
}

param ::= VEC2(A). {
	Vec2 val = std::any_cast<Vec2>(*A);
	if(!machine.TryPushReg(val)) {
		*valid = false;
	}

	delete A;
}

param ::= NUMBER_LITERAL(A). {
	std::cout << "Number literal" << std::endl;
	if(!machine.TryPushReg(std::any_cast<uint32_t>(*A))) {
		*valid = false;
	}

	delete A;
}

param ::= MOD(A). {
	if(!machine.TryApplyModifier(std::any_cast<RegModifier>(*A))) {
		*valid = false;
	}

	delete A;
}

set_register ::= REG(A). {
	if(!machine.TrySetRegister(GenReg(std::any_cast<GifRegisters>(*A)))) {
		*valid = false;
	}

	delete A;
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
