#include "typecheck.hpp"

void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

CompoundType getCompoundType (TypeNode* node) {
  if (node->basetype == bt_object) {
    CompoundType compound_type = {node->basetype, ((ObjectTypeNode*)node)->identifier->name};
    return compound_type;
  }
  else {
    CompoundType compound_type = {node->basetype, ""};
    return compound_type;
  }
}


CompoundType verifyVariable (TypeCheck* obj, IdentifierNode* node, std::string currentClassName, bool local) {
  ClassTable* classTable = obj->classTable;
  VariableTable* currentVariableTable = obj->currentVariableTable;

  if (local && currentVariableTable->find(node->name) != currentVariableTable->end()) {
    node->basetype = (*currentVariableTable)[node->name].type.baseType;
    return (*currentVariableTable)[node->name].type;
  }

  std::string className = currentClassName;
  if (className != "") {
    if ((*classTable)[className].members->find(node->name) != (*classTable)[className].members->end()) {
      node->basetype = (*(*classTable)[className].members)[node->name].type.baseType;
      return (*(*classTable)[className].members)[node->name].type;
    }
    className = (*classTable)[className].superClassName;
  }

  if (local) {
    typeError (undefined_variable);
  }
  else {
    typeError (undefined_member);
  }

  CompoundType c;
  return c;
}

MethodInfo verifyMethodsFromClass(TypeCheck* obj, IdentifierNode* node, std::string currentClassName) {
  ClassTable* classTable = obj->classTable;
  MethodTable* currentMethodTable = obj->currentMethodTable;

  std::string className = currentClassName;
  
  while (className != "") {
    if ((*classTable)[className].methods->find(node->name) != (*classTable)[className].methods->end()) {
      node->basetype = (*(*classTable)[className].methods)[node->name].returnType.baseType;
      return (*(*classTable)[className].methods)[node->name];
    }
    className = (*classTable)[className].superClassName;
  }

  typeError (undefined_method);
  MethodInfo m;
  return m;
}

void TypeCheck::visitProgramNode(ProgramNode* node) {
  classTable = new ClassTable();
  node->visit_children(this);

  if (classTable->find("Main") == classTable->end()) {
    typeError(no_main_class);
  }
}

void TypeCheck::visitClassNode(ClassNode* node) {
  currentMemberOffset = 0;
  currentParameterOffset = 0;
  currentLocalOffset = 0;
  currentClassName = node->identifier_1->name;
  (*classTable)[currentClassName].membersSize = 0;
  currentMethodTable = new MethodTable();
  currentVariableTable = new VariableTable();

  if (node->identifier_2 != NULL && classTable->find(node->identifier_2->name) == classTable->end()){
    typeError(undefined_class);
  }

  std::string superclass = (node->identifier_2) ? node->identifier_2->name : "";
  if (superclass != "") {
    for (auto const& member : (*(*classTable)[superclass].members)) {
      (*currentVariableTable)[member.first] = member.second;
      currentMemberOffset += 4;
    }
  }

  ClassInfo class_info = {superclass, currentMethodTable, currentVariableTable, (int)node->method_list->size() * 4};
  (*classTable)[node->identifier_1->name] = class_info;
  node->visit_children(this);

  (*classTable)[currentClassName].membersSize = currentMemberOffset;
  
  if (node->identifier_1->name == "Main") {
    if (node->declaration_list->size() != 0) {
      typeError(main_class_members_present);
    }

    bool main_found = false;
    auto firstlabel = node->method_list->begin();
    for(; firstlabel != node->method_list->end(); ++firstlabel) {
      if ((*firstlabel)->identifier->name == "main") {
        main_found = true;

        if ((*firstlabel)->parameter_list->size() != 0) {
          typeError(main_method_incorrect_signature);
        }

        if ((*firstlabel)->type->basetype != bt_none) {
          typeError(main_method_incorrect_signature);
        }

      }
    }
    if (!main_found) {
      typeError(no_main_method);
    }
  }
}

void TypeCheck::visitMethodNode(MethodNode* node) {
  currentParameterOffset = 12;
  currentLocalOffset = -4;
  int localSize = 0;

  for (auto iter = node->methodbody->declaration_list->begin(); iter != node->methodbody->declaration_list->end(); ++iter) {
    localSize += 4 * (*iter)->identifier_list->size();
  }

  CompoundType compound_type = getCompoundType(node->type);
  currentVariableTable = new VariableTable();

  std::list<CompoundType>* params = new std::list<CompoundType>();
  for (auto iter = node->parameter_list->begin(); iter != node->parameter_list->end(); ++iter) {
    CompoundType compound_type = getCompoundType((*iter)->type);
    params->push_back(compound_type);
  }

  MethodInfo method_info = {compound_type, currentVariableTable, params, localSize};
  (*currentMethodTable)[node->identifier->name] = method_info;

  node->visit_children(this);

  if (node->methodbody->returnstatement == NULL && method_info.returnType.baseType != bt_none) {
    typeError(return_type_mismatch);
  }

  if (node->methodbody->returnstatement != NULL && node->methodbody->returnstatement->basetype != method_info.returnType.baseType) {
    typeError(return_type_mismatch);
  }
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  node->visit_children(this);
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  VariableInfo var_info = {getCompoundType (node->type), currentParameterOffset, 4};
  currentParameterOffset += 4;
  (*currentVariableTable)[node->identifier->name] = var_info;

  node->visit_children(this);
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  node->visit_children(this);
  bool localVars = (currentLocalOffset != 0);
  if (node->type->basetype == bt_object && classTable->find(  ((ObjectTypeNode*)node->type)->identifier->name  ) == classTable->end()) {
      typeError(undefined_class);
  }

  for (auto iter = node->identifier_list->begin(); iter != node->identifier_list->end(); ++iter) {
    VariableInfo var_info;
    if (localVars) {
      var_info = {getCompoundType(node->type), currentLocalOffset, 4};
      currentLocalOffset -= 4;
    }
    else {
      var_info = {getCompoundType(node->type), currentMemberOffset, 4};
      currentMemberOffset += 4;
    }
    (*currentVariableTable)[((*iter)->name)] = var_info;
  }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  node->visit_children(this);
  node->basetype = node->expression->basetype;
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  node->visit_children(this);
  if (node->identifier_2 == NULL) {
    CompoundType c1 = verifyVariable (this, node->identifier_1, currentClassName, true);
    BaseType c2 = node->expression->basetype;

    if (c1.baseType != c2) {
      typeError (assignment_type_mismatch);
    }
  }
  else {
    CompoundType c1 = verifyVariable (this, node->identifier_1, currentClassName, true);

    if (c1.baseType != bt_object) {
      typeError (not_object);
    }

    CompoundType c2 = verifyVariable (this, node->identifier_2, c1.objectClassName, false);

    BaseType c3 = node->expression->basetype;

    if (c2.baseType != c3) {
      typeError (assignment_type_mismatch);
    }
  }
}

void TypeCheck::visitCallNode(CallNode* node) {
  node->visit_children(this);
}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  node->visit_children(this);
  
  if (node->expression->basetype != bt_boolean) {
    typeError(if_predicate_type_mismatch);
  }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  node->visit_children(this);
  
  if (node->expression->basetype != bt_boolean) {
    typeError(while_predicate_type_mismatch);
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  node->visit_children(this);

  if (node->expression->basetype != bt_boolean) {
    typeError(do_while_predicate_type_mismatch);
  }
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  node->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer ||
      node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }

  node->basetype = node->expression_1->basetype;
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer ||
      node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }

  node->basetype = node->expression_1->basetype;
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer ||
      node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }

  node->basetype = node->expression_1->basetype;
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype != bt_integer ||
      node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }

  node->basetype = node->expression_1->basetype;
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer) {
    typeError (expression_type_mismatch);
  }
  if (node->expression_2->basetype != bt_integer) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer) {
    typeError (expression_type_mismatch);
  }
  if (node->expression_2->basetype != bt_integer) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype != node->expression_2->basetype) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode* node) {
  node->visit_children(this);
  if (node->expression_1->basetype != bt_boolean) {
    typeError (expression_type_mismatch);
  }
  if (node->expression_2->basetype != bt_boolean) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) {
  node->visit_children(this);
  if (node->expression_1->basetype != bt_boolean) {
    typeError (expression_type_mismatch);
  }
  if (node->expression_2->basetype != bt_boolean) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode* node) {
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  node->visit_children(this);
  if (node->expression->basetype != bt_integer) {
    typeError (expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  node->visit_children(this);
  if (node->identifier_2 == NULL) {
    MethodInfo mi = verifyMethodsFromClass(this, node->identifier_1, currentClassName);

    if (node->expression_list->size() != mi.parameters->size()) {
      typeError (argument_number_mismatch);
    }

    auto firstlabel = node->expression_list->begin();
    auto secondlabel = mi.parameters->begin();
    for(; firstlabel != node->expression_list->end(); ++firstlabel, ++secondlabel) {
      if ((*firstlabel)->basetype != secondlabel->baseType) {
        typeError (argument_type_mismatch);
      }
    }

    node->basetype = mi.returnType.baseType;
  }
  else {
    CompoundType c = verifyVariable (this, node->identifier_1, currentClassName, true);

    if (c.baseType != bt_object) {
      typeError (not_object);
    }

    MethodInfo mi = verifyMethodsFromClass (this, node->identifier_2, c.objectClassName);

    if (node->expression_list->size() != mi.parameters->size()) {
      typeError (argument_number_mismatch);
    }

    auto firstlabel = node->expression_list->begin();
    auto secondlabel = mi.parameters->begin();
    for(; firstlabel != node->expression_list->end(); ++firstlabel, ++secondlabel) {
      if ((*firstlabel)->basetype != secondlabel->baseType) {
        typeError (argument_type_mismatch);
      }
    }

    node->basetype = mi.returnType.baseType;
  }
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  CompoundType c1 = verifyVariable (this, node->identifier_1, currentClassName, true);

  if (c1.baseType != bt_object) {
    typeError (not_object);
  }

  CompoundType c2 = verifyVariable (this, node->identifier_2, c1.objectClassName, false);
  node->basetype = c2.baseType;
}

void TypeCheck::visitVariableNode(VariableNode* node) {
  CompoundType c = verifyVariable (this, node->identifier, currentClassName, true);
  node->basetype = c.baseType;
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  node->basetype = bt_boolean;
}

void TypeCheck::visitNewNode(NewNode* node) {
  node->visit_children(this);

  std::string className = node->identifier->name;
  
  if (classTable->find(className) == classTable->end()) {
    typeError(undefined_class);
  }

  ClassInfo class_info = (*classTable)[className];

  if (class_info.methods->find(className) == class_info.methods->end()) {
    if (node->expression_list != NULL && node->expression_list->size() != 0) {
      typeError (argument_number_mismatch);
    }
  }
  else {
    MethodInfo constructor = (*class_info.methods)[className];

    if (node->expression_list->size() != constructor.parameters->size()) {
      typeError (argument_number_mismatch);
    }

    auto firstlabel = node->expression_list->begin();
    auto secondlabel = constructor.parameters->begin();
    for(; firstlabel != node->expression_list->end(); ++firstlabel, ++secondlabel) {
      if ((*firstlabel)->basetype != secondlabel->baseType) {
        typeError (argument_type_mismatch);
      }
    }

  }

  node->basetype = bt_object;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  node->basetype = bt_boolean;
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  node->basetype = bt_object;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  node->basetype = bt_none;
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
}


std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
