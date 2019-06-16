#include <stdio.h>
#include <string.h>
#include "./parsingJVM.h"

void writeJasminFile(char* str) {
  FILE* fp;
  fp = fopen("hw3.j", "a+");
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
    strcat(buf, "S");
  } else {
    strcat(buf, "V");
  }
}


void outputFunctionDef(char* name, char* argumentsType, char* returnType) {
  // printf("%s %s %s \n", name, argumentsType, returnType);
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

void outputVariableDef(char* name, char* type, char* value, int scope, int reg) {
  // printf("%s %s %s \n", name, type, value);
  char str[512] = {0};
  char typestr[16] = {0};

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
    // 0 is the default value if the value is null
    sprintf(load, "\tldc %s\n", value ? value : "0");
    strcat(str, load);
    char* store = strcmp(type, "float") == 0 ? "fstore" : "istore";
    char store1[32] = {0};
    sprintf(store1, "\t%s %d\n", store, reg);
    strcat(str, store1);
  }
  writeJasminFile(str);
}

void outputVariable(char* target, int reg, int scope, char* type, int which, char* target1, int reg1, int scope1, char* type1, int which1, char* op, char* exprType) {
  char str[512] = {0};
  char doOp[16] = {0};
  determineVariable(target, reg, scope, type, type1, which, str);
  determineVariable(target1, reg1, scope1, type1, type, which1, str);
  // If one of the operands is float
  if (strcmp(type, "float") == 0 || strcmp(type1, "float") == 0) {
    strcpy(exprType, "float");
    sprintf(doOp, "\t%c", 'f');
  } else {
    strcpy(exprType, "int");
    sprintf(doOp, "\t%c", 'i');
  }
  printf("op: %s\n", op);
  // Check for operation
  if (strcmp(op, "+") == 0) {
    strcat(doOp, "add\n");
  } else if(strcmp(op, "-") == 0) {
    strcat(doOp, "sub\n");
  } else if(strcmp(op, "*") == 0) {
    strcat(doOp, "mul\n");
  } else if(strcmp(op, "/") == 0) {
    strcat(doOp, "div\n");
  } else {
    strcat(doOp, "rem\n");
  }

  strcat(str, doOp);
  writeJasminFile(str);
}

void determineVariable(char* target, int reg, int scope, char* type, char* type1, int which, char* str) {
  // Load
  char load[128] = {0};
  switch(which) {
    case 1: { // const
      char* loadOp = "ldc";
      sprintf(load, "\t%s %s\n", loadOp, target);
      printf("load a constant: %s", load);
      // check for casting
      if (strcmp(type, type1) != 0) {
        // printf("type: %s %s\n", type, type1);
        if (strcmp(type, "int") == 0) {
          // printf("who is the target: %s\n", target);
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

        // check for casting
        if (strcmp(type, type1) != 0) {
          // printf("type: %s %s\n", type, type1);
          if (strcmp(type, "int") == 0) {
            // printf("who is the target: %s\n", target);
            strcat(load, "\ti2f\n");
          }
        }
      } else { // global
        char* loadOp = "getstatic compiler_hw3/";
        char tmp[5] = {0};
        resolveType(type, tmp);
        sprintf(load, "\t%s%s %s\n", loadOp, target, tmp);
        printf("load global: %s", load);
         // check for casting
        if (strcmp(type, type1) != 0) {
          // printf("type: %s %s\n", type, type1);
          if (strcmp(type, "int") == 0) {
            // printf("who is the target: %s\n", target);
            strcat(load, "\ti2f\n");
          }
        }
      }
    }
  }
  strcat(str, load);
}

void outputAssignment(char* leftType, int leftReg, char* rightType, char* op) {
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

  } else { // local
    sprintf(s, "\t%cstore %d\n", strcmp(leftType, "float") == 0 ? 'f' : 'i', leftReg);
  }
  strcat(str, s);
  writeJasminFile(str);
}