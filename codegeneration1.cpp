#include "codegeneration.hpp"

// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.




void CodeGenerator::visitProgramNode(ProgramNode* node) {
	// WRITEME: Replace with code if necessary
	std::cout << ".data" << std::endl;
	std::cout << "printstr: .asciz \"%d\\n\"" << std::endl;
	std::cout << ".text" << std::endl;
	std::cout << ".globl Main_main" << std::endl;
	node->visit_children(this);

}

void CodeGenerator::visitClassNode(ClassNode* node) {
	currentClassName = node->identifier_1->name;
	currentClassInfo = classTable->at(currentClassName);
	node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
	currentMethodName = node->identifier->name;
	currentMethodInfo = classTable->at(currentClassName).methods->at(currentMethodName);
	std::cout << currentClassName << "_" << currentMethodName << ": " << std::endl;

	node->methodbody->accept(this);

}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
	std::cout << "  #Method Start" << std::endl;
	std::cout << "  push %ebp" << std::endl;
	std::cout << "  mov %esp, %ebp" << std::endl;
	std::string offset = std::to_string(currentMethodInfo.localsSize);
	std::cout << "  sub $"<< offset  <<", %esp" << std::endl;
	std::cout << "  push %edi" << std::endl;
	std::cout << "  push %esi" << std::endl;
	std::cout << "  push %ebx" << std::endl;
	std::cout << "  #Method done" << std::endl;

	node->visit_children(this);

	bool constructor = (currentMethodName == currentClassName);

	if (constructor) std::cout << "mov 8(%ebp), %eax" << std::endl;

	std::cout << "  #Clean up" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  pop %esi" << std::endl;
	std::cout << "  pop %edi" << std::endl;
	std::cout << "  mov %ebp, %esp" << std::endl;
	std::cout << "  pop %epb" << std::endl;
	std::cout << "  ret"<< std::endl;
	std::cout << "  #clean up done" << std::endl;

}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
	std::cout << "  #Return Statement" << std::endl;
	std::cout << "  pop %eax" << std::endl;
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
	node->visit_children(this);

	std::cout << "  # VISIT ASSIGNMENT NODE" << std::endl;
	std::cout << "  pop %eax" << std::endl;

	if (node->identifier_2 == NULL) {
		int offset;
		if (currentMethodInfo.variables->count(node->identifier_1->name) > 0) {
			offset = currentMethodInfo.variables->at(node->identifier_1->name).offset;

			std::cout << "  # " << node->identifier_1->name << " #" << std::endl;
		  std::cout << "  # localOffset = " << offset << std::endl;
			std::cout << "  mov %eax, " << offset << "(%ebp)" << std::endl;
		} else {
			offset = currentClassInfo.members->at(node->identifier_1->name).offset;

			int memberOffset = currentClassInfo.members->at(node->identifier_1->name).offset;

			std::cout << "  # memberOffset = " << memberOffset << std::endl;
			std::cout << "  mov 8(%ebp), %ebx" << std::endl;
			std::cout << "  mov %eax, " << memberOffset << "(%ebx)" << std::endl;
		}


	} else {
		int memberOffset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;

		int localOffset = 0;
		if (currentMethodInfo.variables->count(node->identifier_1->name) > 0) {
			localOffset = currentMethodInfo.variables->at(node->identifier_1->name).offset;

			std::cout << "  mov "  << localOffset << "(%ebp), %ebx" << std::endl;
			std::cout << "  mov %eax, " << memberOffset << "(%ebx)" << std::endl;

		} else {

			localOffset = currentClassInfo.members->at(node->identifier_1->name).offset;

			std::cout << "  mov 8(%ebp), %ebx" << std::endl;
			std::cout << "  mov " << localOffset << "(%ebx), %eax" << std::endl;
			std::cout << "  mov %eax, " << memberOffset << "(%ebx)" << std::endl;
		}


	}

	std::cout << "  # VISIT ASSIGNMENT NODE DONE" << std::endl;

}

void CodeGenerator::visitCallNode(CallNode* node) {
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {

	std::string label = std::to_string(nextLabel());

	node->expression->accept(this);

	std::cout << "  # IfElse" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  cmp $1, %eax" << std::endl;
	std::cout << "  jne elseLabel" << label << std::endl;

	if (node->statement_list_1){
		auto i = node->statement_list_1->begin();
		for(i; i != node->statement_list_1->end(); i++) {
			(*i)->accept(this);
		}
	}

	std::cout << "  jmp endLabel" << label << std::endl;
	std::cout << "  elseLabel" << label << ":" << std::endl;

	if (node->statement_list_2){
		auto i = node->statement_list_2->begin();
		for(i; i != node->statement_list_2->end(); i++){
			(*i)->accept(this);
		}
	}

	std::cout << "  endLabel" << label << ":" << std::endl;

}

void CodeGenerator::visitWhileNode(WhileNode* node) {
	std::string label = std::to_string(nextLabel());

	std::cout << "  # While" << std::endl;
	std::cout << "  whileLabel" << label << ":" << std::endl;

	node->expression->accept(this);

	std::cout << "  pop %eax" << std::endl;
	std::cout << "  cmp $1, %eax" << std::endl;
	std::cout << "  jne endWhileLabel" << label << std::endl;

	if (node->statement_list){
		auto i = node->statement_list->begin();
		for(i; i != node->statement_list->end(); i++) {
			(*i)->accept(this);
		}
	}

	std::cout << "  jmp topWhileLabel" << label << std::endl;
	std::cout << "  endWhileLabel" << label << ":" << std::endl;
}

void CodeGenerator::visitPrintNode(PrintNode* node) {

	node->visit_children(this);

	std::cout << "  # print " << std::endl;
	std::cout << "  push $printstr" << std::endl;
	std::cout << "  call printf" << std::endl;
	std::cout << "  add $8, %esp" << std::endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
	int l1 = nextLabel();

	std::cout << "  # DoWhile" << std::endl;
	std::cout << "  L" << l1 << ":" << std::endl;

	auto i = node->statement_list->begin();
	for(; i != node->statement_list->end(); ++i) {
		(*i)->accept(this);
	}

	node->expression->accept(this);

	std::cout << "  pop %edx" << std::endl;
	std::cout << "  mov $1, %eax" << std::endl;
	std::cout << "  cmp %edx, %eax" << std::endl;
	std::cout << "  je L" << l1 << std::endl;
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
	node->visit_children(this);
	std::cout << "  # Plus" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  add %ebx, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
	node->visit_children(this);
	std::cout << "  # Subtract" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  sub %ebx, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
	node->visit_children(this);
	std::cout << "  # Times" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  imul %ebx, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
	node->visit_children(this);
	std::cout << "  # Divide" << std::endl;
	std::cout << "  pop %ecx" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  cdq" << std::endl;
	std::cout << "  idiv %ecx" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
	node->visit_children(this);
	int l1 = nextLabel();
	int l2 = nextLabel();

	std::cout << "  # Greater" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  cmp %eax, %ebx" << std::endl;
	std::cout << "  jg L" << l1 << std::endl;
	std::cout << "  push $0" << std::endl;
	std::cout << "  jmp L" << l2 << std::endl;
	std::cout << "  L" << l1 << ": push $1" << std::endl;
	std::cout << "  L" << l2 << ": " << std::endl;
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
	node->visit_children(this);
	int l1 = nextLabel();
	int l2 = nextLabel();

	std::cout << "  # GreaterEqual" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  cmp %eax, %ebx" << std::endl;
	std::cout << "  jge L" << l1 << std::endl;
	std::cout << "  push $0" << std::endl;
	std::cout << "  jmp L" << l2 << std::endl;
	std::cout << "  L" << l1 << ": push $1" << std::endl;
	std::cout << "  L" << l2 << ": " << std::endl;
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
	node->visit_children(this);

	std::cout << "  # Equal" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  cmp %eax, %ebx" << std::endl;
	std::cout << "  sete %d1" <<  std::endl;
	std::cout << "  movzb1 %d1, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;

}

void CodeGenerator::visitAndNode(AndNode* node) {
	node->visit_children(this);
	std::cout << "  # And" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  and %ebx, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitOrNode(OrNode* node) {
	node->visit_children(this);
	std::cout << "  # Or" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  pop %ebx" << std::endl;
	std::cout << "  or %ebx, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitNotNode(NotNode* node) {
	node->visit_children(this);
	std::cout << "  # Not" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  mov $1, %ebx" << std::endl;
	std::cout << "  xor %ebx, %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
	node->visit_children(this);
	std::cout << "  # Negation" << std::endl;
	std::cout << "  pop %eax" << std::endl;
	std::cout << "  neg %eax" << std::endl;
	std::cout << "  push %eax" << std::endl;
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
	int argCnt = 0;

	if (node->expression_list != NULL) {
		auto i = node->expression_list->rbegin();
		for(; i != node->expression_list->rend(); ++i) {
			(*i)->accept(this);
			argCnt++;
		}
	}

	std::string className = "";
	std::string methodName = "";

	if (node->identifier_2 == NULL) {
		while (classTable->at(currentClassName).methods->count(node->identifier_1->name) == 0) {
			className = classTable->at(className).superClassName;
		}
		std::cout << "  push 8(%ebp)" << std::endl;
	} else {
		className = node->identifier_1->objectClassName;
		methodName = node->identifier_2->name;

		if (currentMethodInfo.variables->count(node->identifier_1->name) > 0) {
				std::cout << "  push " << currentMethodInfo.variables->at(node->identifier_1->name).offset << "(%ebp)" << std::endl;
		} else {
				std::cout << "  mov 8(%ebp), %ebx" << std::endl;
				std::cout << "  push " << currentClassInfo.members->at(node->identifier_1->name).offset << "(%ebx)" << std::endl;
		}

		while (classTable->at(className).methods->count(methodName) == 0) {
			className = classTable->at(className).superClassName;
		}
	}

		std::cout << "  call " << className << "_" << methodName << std::endl;
		std::cout << "  add $" << argCnt * (4*4) << ", %esp" << std::endl;
		std::cout << "  push %eax" << std::endl;
		std::cout << "  # METHOD CALL END" << std::endl;
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
	node->visit_children(this);

	std::cout << "  # VISIT MEMBER ACCESS" << std::endl;

	int localOffset;
	int memberOffset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;

	if (currentMethodInfo.variables->count(node->identifier_1->name) > 0) {
		localOffset = currentMethodInfo.variables->at(node->identifier_1->name).offset;

		std::cout << "  mov " << localOffset << "(%ebp), %ebx" << std::endl;
		std::cout << "  mov " << memberOffset << "(%ebx), %eax" << std::endl;
		std::cout << "  push %eax" << std::endl;
	} else {
		localOffset = currentClassInfo.members->at(node->identifier_1->name).offset;
		std::cout << "  mov 8(%ebp), %ebx" << std::endl;
		std::cout << "  mov " << localOffset << "(%ebx), %ecx" << std::endl;
		std::cout << "  mov " << memberOffset << "(%ecx), %eax" << std::endl;
		std::cout << "  push %eax" << std::endl;
	}

	std::cout << "  # VISIT MEMBER ACCESS DONE" << std::endl;
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
	node->visit_children(this);

	std::cout << "  # VISIT VARIABLE" << std::endl;

	if (currentMethodInfo.variables->count(node->identifier->name) > 0) {
		int offset = currentMethodInfo.variables->at(node->identifier->name).offset;

		std::cout << "  #  VARIABLE LOCAL" << std::endl;
		std::cout <<"  push " << offset << "(%ebp)" << std::endl;

	} else if (currentClassInfo.members->count(node->identifier->name) > 0) {
		int offset = currentClassInfo.members->at(node->identifier->name).offset;

		std::cout << "  #  VARIABLE MEMBER" << std::endl;
		std::cout << "  mov 8(%ebp), %ebx" << std::endl;
		std::cout << "  mov " << offset << "(%ebx), %eax" << std::endl;
		std::cout << "  push %eax" << std::endl;
	}
	std::cout << "  # VISIT VARIABLE DONE" << std::endl;
}


void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
	node->visit_children(this);
	std::cout << "  push $" << node->integer->value << std::endl;
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
	node->visit_children(this);
	std::cout << "  push $" << node->integer->value << std::endl;
}

void CodeGenerator::visitNewNode(NewNode* node) {
	node->visit_children(this);
	int size = classTable->at(node->identifier->name).membersSize;
	int constructorCnt = classTable->at(node->identifier->name).methods->count(node->identifier->name);

	if (constructorCnt > 0) {
		int argumentCnt = 0;
		if (node->expression_list != NULL){
			auto i = node->expression_list->rbegin();
			for(i; i != node->expression->rend(); i++){
				(*i)->accept(this);
				argumentCnt += 1;
			}
		}
		std::cout << "  push $" << size << std::endl;
		std::cout << "  call malloc" << std::endl;
		std::cout << "  add $4, %esp" << std::endl;
		std::cout << "  push %eax" << std::endl;
		std::cout << "  call " << node->identifier->name << "_" << node->identifier->name << std::endl;
		std::cout << "  add $" << argumentCnt * (4*4) << ", %esp" << std::endl;
		std::cout << "  push %eax" << std::endl;
	}else {
		std::cout << "  # NewNode" << std::endl;
		std::cout << "  push $" << size << std::endl;
		std::cout << "  call malloc" << std::endl;
		std::cout << "  add $4, %esp" << std::endl;
		std::cout << "  push %eax" << std::endl;
	}
}

// No need to do anything below
void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode* node) {
}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode* node) {
}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode* node) {
}

void CodeGenerator::visitNoneNode(NoneNode* node) {
}

void CodeGenerator::visitIdentifierNode(IdentifierNode* node) {
}

void CodeGenerator::visitIntegerNode(IntegerNode* node) {
}
