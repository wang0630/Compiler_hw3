#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "branching.h"

// global top
int top = 0;
// For branching stack
branchingNode* branchingStack[STACK_SIZE];

void stackInit() {
  for (int i = 0; i < STACK_SIZE; i++) {
    branchingStack[i] = (branchingNode*)malloc(sizeof(branchingNode));
  }
}

void stackClear() {
  for (int i = 0; i < STACK_SIZE; i++) {
    free(branchingStack[i]);
  }
}

int topID() {
  return branchingStack[top]->id;
}

void push(char* br, int id) {
  branchingStack[top]->id = id;
  strcpy(branchingStack[top]->br, br);
  top++;
}

branchingNode* pop() {
  return branchingStack[top--];
}
