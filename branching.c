#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "branching.h"

// global top
int top = -1;
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
  if (top < 0) {
    return -1;
  }
  return branchingStack[top]->id;
}

char* topBr() {
   if (top < 0) {
    return NULL;
  }
  return branchingStack[top]->br;
}

void push(char* br, int id) {
  // stack is empty
  if (top < 0) {
    top++;
  }
  branchingStack[top]->id = id;
  strcpy(branchingStack[top]->br, br);
}

branchingNode* pop() {
  return branchingStack[top--];
}
