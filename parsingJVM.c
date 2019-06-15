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
  printf("%s %s %s \n", name, argumentsType, returnType);
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
  printf("%s %s %s \n", name, type, value);
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