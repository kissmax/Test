#include "codegeneration.hpp"

using namespace std;

void CodeGenerator::visitProgramNode(ProgramNode* node) {
	cout << ".data" << endl;
	cout << "printstr: .asciz \"%d\\n\"" << endl;
	cout << ".text" << endl;
	cout << ".globl Main_main" << endl;
	node->visit_children(this);
}

void CodeGenerator::visitClassNode(ClassNode* node) {
	currentClassName = node->identifier_1->name;
	node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
	currentMethodName = node->identifier->name;
	cout << currentClassName << "_" << currentMethodName << ": " << endl;
	cout << "  push %ebp" << endl;
	cout << "  mov %esp, %ebp" << endl;
	int offset = 4*(*(*classTable)[currentClassName].methods)[currentMethodName].localsSize;
	cout << "  sub $"<< offset  <<", %esp" << endl;
	cout << "  push %ebx" << endl;
	cout << "  push %esi" << endl;
	cout << "  push %edi" << endl;
	node->visit_children (this);
	cout << "  pop %edi" << endl;
	cout << "  pop %esi" << endl;
	cout << "  pop %ebx" << endl;
	cout << "  add $" << offset << ", %esp"<< endl;
	cout << "  mov %ebp, %esp" << endl;
	cout << "  pop %ebp" << endl;
	cout << "  ret" << endl;
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
	node->visit_children(this);
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
	node->visit_children(this);
	cout << "  pop %eax" << endl;
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
	node->visit_children (this);
	if (node->identifier_2 == NULL) {
		auto localTable = (*(*(*classTable)[currentClassName].methods)[currentMethodName].variables);
		if (localTable.find(node->identifier_1->name) != localTable.end()) {
			int offset = localTable[node->identifier_1->name].offset;
			cout << "  pop %eax" << endl;
			cout << "  mov %eax, " << offset << "(%ebp)" << endl;
		}
		else {
			int offset = (*(*classTable)[currentClassName].members)[node->identifier_1->name].offset;
			cout << "  mov 8(%ebp), %ebx" << endl;
			cout << "  pop %eax"  << endl;
			cout << "  mov %eax, " << offset << "(%ebx)" << endl;
		}
	}
	else {
		auto localVars = (*(*(*classTable)[currentClassName].methods)[currentMethodName].variables);
		if (localVars.find(node->identifier_1->name) != localVars.end()) {
			auto className = localVars[node->identifier_1->name].type.objectClassName;
			int offset = (*(*classTable)[className].members)[node->identifier_2->name].offset;
			int localVarOffset = localVars[node->identifier_1->name].offset;
			cout << "  mov " << localVarOffset << "(%ebp), %eax" << endl;
			cout << "  pop %edx" << endl;
			cout << "  mov %edx, " << offset << "(%eax)" << endl;
		}
		else {
			auto className = (*(*classTable)[currentClassName].members)[node->identifier_1->name].type.objectClassName;
			int offset = (*(*classTable)[className].members)[node->identifier_2->name].offset;
			auto memberOff = (*(*classTable)[className].members)[node->identifier_1->name].offset;
			cout << "  mov 8(%ebp), %eax" << endl;
			cout << "  mov " << memberOff << "(%eax), %eax" << endl;
			cout << "  pop %edx" << endl;
			cout << "  mov %edx, " << offset <<  "(%eax)" << endl;
		}
	}
}

void CodeGenerator::visitCallNode(CallNode* node) {
	node->visit_children(this);
	cout << "  add $4, %esp" << endl;
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
	int firstlabel = nextLabel();
	int secondlabel = nextLabel();
	node->expression->accept(this);
	cout << "  pop %edx" << endl;
	cout << "  mov $1, %eax" << endl;
	cout << "  cmp %edx, %eax" << endl;
	cout << "  je L" << firstlabel << endl;
	if (node->statement_list_2 != NULL){
		auto i = node->statement_list_2->begin();
		for(; i != node->statement_list_2->end(); ++i) {
			(*i)->accept(this);
		}
	}
	cout << "  jmp L" << secondlabel << endl;
	cout << "  L" << firstlabel << ":" << endl;
	auto j = node->statement_list_1->begin();
	for(; j != node->statement_list_1->end(); ++j) {
		(*j)->accept(this);
	}
	cout << "  L" << secondlabel << ":" << endl;
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
	int firstlabel = nextLabel();
	int secondlabel = nextLabel();
	cout << "  L" << firstlabel << ":" << endl;
	node->expression->accept(this);
	cout << "  pop %edx" << endl;
	cout << "  mov $0, %eax" << endl;
	cout << "  cmp %edx, %eax" << endl;
	cout << "  je L" << secondlabel << endl;
	auto i = node->statement_list->begin();
	for(; i != node->statement_list->end(); ++i) {
		(*i)->accept(this);
	}
	cout << " jmp L" << firstlabel << endl;
	cout << "  L" << secondlabel << ":" << endl;
}

void CodeGenerator::visitPrintNode(PrintNode* node) {
	cout << "  push %eax" << endl;
	cout << "  push %ecx" << endl;
	cout << "  push %edx" << endl;
	node->visit_children(this);
	cout << "  push $printstr" << endl;
	cout << "  call printf" << endl;
	cout << "  add $4, %esp" << endl;
	cout << "  pop %edx" << endl;
	cout << "  pop %ecx" << endl;
	cout << "  pop %eax" << endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
	int firstlabel = nextLabel();
	cout << "  # DoWhile" << endl;
	cout << "  L" << firstlabel << ":" << endl;
	auto i = node->statement_list->begin();
	for(; i != node->statement_list->end(); ++i) {
		(*i)->accept(this);
	}
	node->expression->accept(this);
	cout << "  pop %edx" << endl;
	cout << "  mov $1, %eax" << endl;
	cout << "  cmp %edx, %eax" << endl;
	cout << "  je L" << firstlabel << endl;
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
	node->visit_children(this);
	cout << "  # Plus" << endl;
	cout << "  pop %edx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  add %edx, %eax" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
	node->visit_children(this);
	cout << "  pop %edx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  sub %edx, %eax" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
	node->visit_children(this);
	cout << "  pop %eax" << endl;
	cout << "  pop %ecx" << endl;
	cout << "  imul %ecx" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
	node->visit_children(this);
	cout << "  # Divide" << endl;
	cout << "  pop %ecx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  cdq" << endl;
	cout << "  idiv %ecx" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
	node->visit_children(this);
	int firstlabel = nextLabel();
	int secondlabel = nextLabel();
	cout << "  pop %eax" << endl;
	cout << "  pop %edx" << endl;
	cout << "  cmp %eax, %edx" << endl;
	cout << "  jg L" << firstlabel << endl;
	cout << "  push $0" << endl;
	cout << "  jmp L" << secondlabel << endl;
	cout << "  L" << firstlabel << ": push $1" << endl;
	cout << "  L" << secondlabel << ": " << endl;
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
	node->visit_children(this);
	int firstlabel = nextLabel();
	int secondlabel = nextLabel();
	cout << "  pop %eax" << endl;
	cout << "  pop %edx" << endl;
	cout << "  cmp %eax, %edx" << endl;
	cout << "  jge L" << firstlabel << endl;
	cout << "  push $0" << endl;
	cout << "  jmp L" << secondlabel << endl;
	cout << "  L" << firstlabel << ": push $1" << endl;
	cout << "  L" << secondlabel << ": " << endl;
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
	node->visit_children(this);
	int firstlabel = nextLabel();
	int secondlabel = nextLabel();
	cout << "  pop %edx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  cmp %edx, %eax" << endl;
	cout << "  je L" << firstlabel << endl;
	cout << "  push $0" << endl;
	cout << "  jmp L" << secondlabel << endl;
	cout << "  L" << firstlabel << ": push $1" << endl;
	cout << "  L" << secondlabel << ": " << endl;
}

void CodeGenerator::visitAndNode(AndNode* node) {
	node->visit_children(this);
	cout << "  pop %edx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  and %edx, %eax" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitOrNode(OrNode* node) {
	node->visit_children(this);
	cout << "  pop %edx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  or %edx, %eax" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitNotNode(NotNode* node) {
	node->visit_children(this);
	cout << "  mov $1, %edx" << endl;
	cout << "  pop %eax" << endl;
	cout << "  xor %edx, %eax" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
	node->visit_children(this);
	cout << "  pop %edx" << endl;
	cout << "  mov $0, %eax" << endl;
	cout << "  sub %edx, %eax" << endl;
	cout << "  push %eax" << endl;
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
	cout << "  push %eax" << endl;
	cout << "  push %ecx" << endl;
	cout << "  push %edx" << endl;
	auto i = node->expression_list->rbegin();
	for(; i != node->expression_list->rend(); ++i) {
		(*i)->accept(this);
	}
	if (node->identifier_2 == NULL) {
		cout << "  push 8(%ebp)" <<endl;
		std::string className = currentClassName;
		while (className != "") {
			if ((*classTable)[className].methods->find(node->identifier_1->name) != (*classTable)[className].methods->end()) {
				cout << "  call " << className << "_" << node->identifier_1->name << endl;
				break;
			}
			className = (*classTable)[className].superClassName;
		}
	}
	else {
		auto localVars = (*(*(*classTable)[currentClassName].methods)[currentMethodName].variables);
		std::string className = "";
		if (localVars.find(node->identifier_1->name) != localVars.end()) {
			className = localVars[node->identifier_1->name].type.objectClassName;
			int localVarOffset = localVars[node->identifier_1->name].offset;
			cout << "  push " << localVarOffset << "(%ebp)" << endl;
		}
		else {
			className = (*(*classTable)[currentClassName].members)[node->identifier_1->name].type.objectClassName;
			auto memberOff = (*(*classTable)[className].members)[node->identifier_1->name].offset;
			cout << "  mov 8(%ebp), %eax" << endl;
			cout << "  push " << memberOff << "(%eax)" << endl;
		}
		while (className != "") {
			if ((*classTable)[className].methods->find(node->identifier_2->name) != (*classTable)[className].methods->end()) {
				cout << "  call " << className << "_" << node->identifier_2->name << endl;
				break;
			}
			className = (*classTable)[className].superClassName;
		}
	}

	cout << "  mov %eax, %ecx" << endl;
	cout << "  add $" << 4 * (node->expression_list->size() + 1) << ", %esp" << endl; 
	cout << "  pop %edx" << endl;
	cout << "  mov 4(%esp), %eax" << endl;
	cout << "  mov %ecx, 4(%esp)" << endl;
	cout << "  pop %ecx" << endl;

}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {

        auto localVars = (*(*(*classTable)[currentClassName].methods)[currentMethodName].variables);
        if (localVars.find(node->identifier_1->name) != localVars.end()) {
		auto varInfo = localVars[node->identifier_1->name];
		auto varClassName = varInfo.type.objectClassName;
		int offset = (*(*classTable)[varClassName].members)[node->identifier_2->name].offset;	
                int localVarOffset = localVars[node->identifier_1->name].offset;
                cout << "  mov " << localVarOffset << "(%ebp), %eax" << endl;
                cout << "  push " << offset << "(%eax)" << endl;
        }
	else {
		auto varInfo = ((*(*classTable)[currentClassName].members)[node->identifier_1->name]);
		auto varClassName = varInfo.type.objectClassName;
		int offset = (*(*classTable)[varClassName].members)[node->identifier_2->name].offset;
		auto memberOff = (*(*classTable)[currentClassName].members)[node->identifier_1->name].offset;
		cout << "  mov 8(%ebp), %eax" << endl;
		cout << "  mov " << memberOff << "(%eax), %eax" << endl;
		cout << "  push " << offset <<  "(%eax)" << endl;
	}
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
	auto className = currentClassName;
	auto localVars = (*(*(*classTable)[className].methods)[currentMethodName].variables);
	if (localVars.find(node->identifier->name) != localVars.end()) {
		int localVarOffset = localVars[node->identifier->name].offset;
		cout << "  push " << localVarOffset << "(%ebp)" << endl;
	}
	else {
		auto memberOff = (*(*classTable)[className].members)[node->identifier->name].offset;
		cout << "  mov 8(%ebp), %eax" << endl;
		cout << "  push " << memberOff << "(%eax)" << endl;
	}
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
	cout << "  push $" << node->integer->value << endl;
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
	cout << "  push $" << node->integer->value << endl;
}

void CodeGenerator::visitNewNode(NewNode* node) {
	
	int size = (*classTable)[node->identifier->name].membersSize;
	ClassInfo class_info = (*classTable)[node->identifier->name];

	if (class_info.methods->find(node->identifier->name) == class_info.methods->end()) {
		cout << "  push $" << size << endl;
		cout << "  call malloc" << endl;
		cout << "  add $4, %esp" << endl;
		cout << "  push %eax" << endl;
	}
	else {
		cout << "  push %eax" << endl;
		cout << "  push %ecx" << endl;
		cout << "  push %edx" << endl;
		
		if (node->expression_list != NULL) {
			auto i = node->expression_list->rbegin();
			for(; i != node->expression_list->rend(); ++i) {
				(*i)->accept(this);
			}
		}

		cout << "  push $" << size << endl;
		cout << "  call malloc" << endl;
		cout << "  add $4, %esp" << endl;
		cout << "  push %eax" << endl;
		cout << "  call " << node->identifier->name << "_" << node->identifier->name << endl;
		cout << "  mov (%esp), %ecx" << endl;
		cout << "  add $" << 4 * (node->expression_list->size() + 1) << ", %esp" << endl; 
		cout << "  pop %edx" << endl;
		cout << "  mov 4(%esp), %eax" << endl;
		cout << "  mov %ecx, 4(%esp)" << endl;
		cout << "  pop %ecx" << endl;
	}
}

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
