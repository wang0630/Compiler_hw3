#include <stdio.h>
#include <string.h>
#include "./parsingJVM.h"

void writeJasminFile(char* str) {
  FILE* fp;
  fp = fopen("compiler_hw3.j", "a+");
  fwrite(str, 1, strlen(str), fp);
  fclose(fp);
}

void resolveType(char* target, char* buf) {
  if (strcmp(target, "int") == 0) {
    strcat(buf, "I");
  } else if (strcmp(target, "float") == 0) {
    strcat(buf, "F");
  } else if (strcmp(target, "bool") == 0) {
    strcat(buf, "Z");
  } else if(strcmp(target, "string") == 0){
    strcat(buf, "Ljava/lang/String;");
  } else if(strcmp(target, "Ljava/lang/String;") == 0) {
    strcat(buf, "Ljava/lang/String;");
  } else {
    strcat(buf, "V");
  }
}

/*
  If the input function is main, then replace the argumentsType to [Ljava/lang/String;
  otherwise generate the .method... heading for the function def
*/
// --------------------------------------------------------------------------------
void outputFunctionDef(char* name, char* argumentsType, char* returnType) {
  // printf("in outputVariableDef: %s %s %s \n", name, argumentsType, returnType);
  char str[512] = {0};
  if (strcmp(name, "main") == 0) {
    // Replace the argument list for main only
    strcpy(argumentsType, "[Ljava/lang/String;");
  }
  sprintf(str,
    ".method public static %s(%s)%s\n.limit stack 50\n.limit locals 50\n",
    name,
    argumentsType,
    returnType
  );
  writeJasminFile(str);
}
// --------------------------------------------------------------------------------

void outputVariableDef(char* name, char* type, char* value, int scope, int reg, char* rightType) {
  // printf("in outputVariableDef: %s %s %s \n", name, type, value);
  char str[512] = {0};
  char typestr[32] = {0};

  // Global variable
  if (scope == 0) {
    // Get the type
    resolveType(type, typestr);
    // value is presented, = operator should be used
    if (value) {
      sprintf(str, ".field public static %s %s = %s\n", name, typestr, value);
    } else {
      sprintf(str, ".field public static %s %s\n", name, typestr);
    }
  } else {
    // Local variable
    char load[32] = {0};
    char store1[32] = {0};
    // Determine which load operation should this variable use
    char* store = strcmp(type, "float") == 0 ? "fstore" : "istore";
 
    /*
      rightType is not NULL meaning that the RHS of the declaration is actually a expr
    */
    // ------------------------------------------------------------------------------------
    if (rightType) {
      // variable tpye is not the same as exprtype, cast the expr according to the variable
      if (strcmp(type, rightType) != 0) {
        char* casting = strcmp(type, "float") == 0 ? "\ti2f\n" : "\tf2i\n";
        strcat(str, casting);
      }
      sprintf(store1, "\t%s %d\n", store, reg);
      strcat(str, store1);
    // ------------------------------------------------------------------------------------
    } else {
      // 0 is the default value if value is not presented
      sprintf(load, "\tldc %s\n", value ? value : "0");
      strcat(str, load);
      sprintf(store1, "\t%s %d\n", store, reg);
      strcat(str, store1);
    }
  }
  writeJasminFile(str);
}

void outputVariable(char* target, int reg, int scope, char* type, int which, char* target1, int reg1, int scope1, char* type1, int which1, char* op, char* exprType) {
  char str[512] = {0};
  char doOp[32] = {0};
  determineVariables(target, reg, scope, type, type1, which, str);
  determineVariables(target1, reg1, scope1, type1, type, which1, str);
  
  /*
    If one of the operands is float
    We will use fadd, fsub, fmul, fdiv instead of i-version of those
  */
  // --------------------------------------------------------------------------------
  if (strcmp(type, "float") == 0 || strcmp(type1, "float") == 0) {
    strcpy(exprType, "float");
    sprintf(doOp, "\t%c", 'f');
  } else {
    strcpy(exprType, "int");
    sprintf(doOp, "\t%c", 'i');
  }

  // Check for operation
  if (strcmp(op, "+") == 0 || strcmp(op, "+=") == 0) {
    strcat(doOp, "add\n");
  } else if(strcmp(op, "-") == 0 || strcmp(op, "-=") == 0) {
    strcat(doOp, "sub\n");
  } else if(strcmp(op, "*") == 0 || strcmp(op, "*=") == 0) {
    strcat(doOp, "mul\n");
  } else if(strcmp(op, "/") == 0 || strcmp(op, "/=") == 0) {
    strcat(doOp, "div\n");
  } else {
    strcat(doOp, "rem\n");
  }
  // --------------------------------------------------------------------------------
  strcat(str, doOp);
  writeJasminFile(str);
}

void determineVariables(char* target, int reg, int scope, char* type, char* type1, int which, char* str) {
  // Load
  char load[128] = {0};
  switch(which) {
    case 1: { // const
      char* loadOp = "ldc";
      sprintf(load, "\t%s %s\n", loadOp, target);
      // check for casting
      if (strcmp(type, type1) != 0) {
        if (strcmp(type, "int") == 0) {
          strcat(load, "\ti2f\n");
        }
      }
      break;
    }
    case 2: { // expr
      return;
    }
    case 3: {
      if (scope != 0) {
        char* loadOp = strcmp(type, "int") == 0 ? "iload" : "fload";
        sprintf(load, "\t%s %d\n", loadOp, reg);
        printf("load a local variable: %s", load);

        /*
          check for casting
          if the variable being processed is int and type is different from type1,
          then cast the variable to float
        */
        // --------------------------------------------------------------------------------
        if (strcmp(type, type1) != 0) {
          if (strcmp(type, "int") == 0) {
            strcat(load, "\ti2f\n");
          }
        }
        // --------------------------------------------------------------------------------
      } else {
        // global
        char* loadOp = "getstatic compiler_hw3/";
        char tmp[32] = {0};
        resolveType(type, tmp);
        sprintf(load, "\t%s%s %s\n", loadOp, target, tmp);
        printf("load global: %s", load);
        /*
          check for casting
          if the variable being processed is int and type is different from type1,
          then cast the variable to float
        */
        // --------------------------------------------------------------------------------
        if (strcmp(type, type1) != 0) {
          if (strcmp(type, "int") == 0) {
            strcat(load, "\ti2f\n");
          }
        }
        // --------------------------------------------------------------------------------
      }
    }
  }
  strcat(str, load);
}


void determineAndOutputOneVariable(char* target, int reg, int which, char* type) {
  char str[512] = {0};
  char loadOp[64] = {0};
  switch(which) {
    case 3: // variable
      if(reg == -1) { // global
        strcpy(loadOp, "getstatic compiler_hw3/");
        char tmp[32] = {0};
        resolveType(type, tmp);
        sprintf(str, "\t%s%s %s\n", loadOp, target, tmp);
      } else {
        strcpy(loadOp, strcmp(type, "float") == 0 ? "fload" : "iload");
        sprintf(str, "\t%s %d\n", loadOp, reg);
      }
      break;
    case 1: // const
      sprintf(str, "\tldc %s\n", target);
      break;
    case 4: // function call
      printf("don't do anything in function call\n");
  }
  writeJasminFile(str);
}

void outputAssignment(char* leftType, int leftReg, char* rightType, char* op, char* name) {
  char str[512] = {0};
  if (strcmp(leftType, rightType) != 0 && strcmp(leftType, "bool") != 0) {
    // Do the implicit casting according to the type of the left side operand
    // if a = 3.4;
    // and a is int, we should have f2i
    char convert[8] = {0};
    if (strcmp(leftType, "int") == 0) {
      strcpy(convert, "\tf2i\n");
    } else {
      strcpy(convert, "\ti2f\n");
    }
    strcat(str, convert);
  }
  char s[64] = {0};
  if (leftReg == -1) { // global
    char tmp[32] = {0};
    resolveType(leftType, tmp);
    sprintf(s, "\tputstatic compiler_hw3/%s %s\n", name, tmp);
    // sprintf(str, "\t%s%s %s\n", loadOp, target, tmp);
    
  } else { // local
    sprintf(s, "\t%cstore %d\n", strcmp(leftType, "float") == 0 ? 'f' : 'i', leftReg);
  }
  strcat(str, s);
  writeJasminFile(str);
}

void outputPrintFuc(char* target, char* type) {
  char str[512] = {0};
  char* op = "\tgetstatic java/lang/System/out Ljava/io/PrintStream;\n\tswap\n\tinvokevirtual java/io/PrintStream/println";
  char buf[32] = {0};
  resolveType(type, buf);
  sprintf(str, "%s(%s)V\n", op, buf);
  writeJasminFile(str);
}

void outputFunctionCall(char* target, char* returnType, char* paras) {
  char str[512] = {0};
  if (paras) {
    sprintf(str, "\tinvokestatic compiler_hw3/%s(%s)%s\n", target, paras, returnType);
  } else {
    sprintf(str, "\tinvokestatic compiler_hw3/%s()%s\n", target, returnType);
  }
  writeJasminFile(str);
}

void outputBr(char* op, char* type, char* label) {
  char str[512] = {0};
  char* sub = strcmp(type, "float") == 0 ? "\tfsub\n" : "\tisub\n";
  char r[64] = {0};
  if (strcmp(op, "<") == 0) {
    sprintf(r, "\tiflt %s\n", label);
  } else if (strcmp(op, ">") == 0) {
    sprintf(r, "\tifgt %s\n", label);
  } else if (strcmp(op, "<=") == 0) {
    sprintf(r, "\tifle %s\n", label);
  } else if (strcmp(op, ">=") == 0) {
    sprintf(r, "\tifge %s\n", label);
  } else if (strcmp(op, "==") == 0) {
    sprintf(r, "\tifeq %s\n", label);
  } else if (strcmp(op, "!=") == 0) {
    sprintf(r, "\tifne %s\n", label);
  }
  sprintf(str, "%s%s", sub, r);
  // printf("%s", str);
  writeJasminFile(str);
}

void convertParameters(char* target, char* buf) {
  char* ptr = strtok(target, ", ");
  while(ptr) {
    char tmp[32] = {0};
    resolveType(ptr, tmp);
    strcat(buf, tmp);
    ptr = strtok(NULL, ", ");
  }
}

void makeLabel(char* buf, char* label, int id) {
  sprintf(buf, "%s%d", label, id);
  char op[32] = {0};
  sprintf(op, "%s:\n", buf);
  writeJasminFile(op);
}