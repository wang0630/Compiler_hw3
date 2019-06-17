void writeJasminFile(char*);
void resolveType(char*, char*);
void determineVariables(char* target, int reg, int scope, char* type, char* type1, int which, char* str);
void determineAndOutputOneVariable(char* target, int reg, int which, char* type);
void outputFunctionDef(char*, char*, char*);
void outputVariableDef(char* name, char* type, char* value, int scope, int reg);
void outputVariable(char* target, int reg, int scope, char* type, int which, char* target1, int reg1, int scope1, char* type1, int which1, char* op, char* exprType);
void outputAssignment(char* leftType, int leftReg, char* rightType, char* op);
void outputPrintFuc(char* target, char* type);