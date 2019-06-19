#ifndef BRANCHING_H
#define BRANCHING_h
#define STACK_SIZE 10

// 0 for while, 1 for if
typedef struct branching {
  char br[16];
  int id;
} branchingNode;

void stackInit();
void stackClear();
void push(char* br, int id);
int topID();
branchingNode* pop();

#endif