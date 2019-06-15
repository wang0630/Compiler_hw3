/*	Definition section */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./parsingJVM.h"
extern int yylineno;
extern int yylex();
extern void yyerror(char*);
extern char* yytext;   // Get current token from lex
extern char buf[256];  // Get current code line from lex

/* Symbol table function - you can add new function if needed. */
// void yyerror(char*);
void lookup_symbol(char*, bool, bool);
void create_symbol();
int lookupRegForInsertion(int);
int lookupRegForJasmin(char*, int);
void lookupVariable (int*, int*, char*, int)
void insert_symbol(char*, char*, char*, int, bool);
void dump_symbol(int);
void makeError(bool, bool);
void yySemError(char*);

/* struct */
typedef struct variable {
    char name[32];
    char entryType[16];
    char myType[16];
    int reg;
    int scope;
    char* formalParameters;
    struct variable* next;
    bool isDeclaredForward;
} variable_t;

// my table
variable_t* symbolTable[10];
char* parameters = NULL;
char errorMsg[100] = {0};
char errorId[32] = {0};
int currentScope = 0;
bool isError = false;
bool shouldDumpNow = false;

// For function definiton
char argumentsType[32] = {0};
// travser through the list
#define TRAVERSE_SINGLE_LIST(i) for (variable_t* ptr = symbolTable[i]; ptr; ptr = ptr->next)
#define TRAVERSE_SINGLE_LIST_NEW(i) for (variable_t* ptr = symbolTable[i]->next; ptr; ptr = ptr->next)
#define TRAVERSE_TABLE for (int i = 0; i < 10; i++)
%}

/* Use variable or self-defined structure to represent
 * nonterminal and token type
 */
%union {
    int i_val;
    double f_val;
    char string[32];
    _Bool bool_val;
}

/* Token without return */
%token ADD SUB MUL DIV MOD INC DEC
%token MT LT MTE LTE EQ NE  
%token ASGN ADDASGN SUBASGN MULASGN DIVASGN MODASGN
%token AND OR NOT
%token LB RB LCB RCB LSB RSB COMMA
%token PRINT 
%token IF ELSE 
%token FOR WHILE
%token ID SEMICOLON QUOTA
%token RET
%token INT FLOAT STRING VOID BOOL

/* Token with return, which need to specify type */
%token <i_val> I_CONST
%token <f_val> F_CONST
%token <string> STR_CONST
%token <bool_val> TRUE
%token <bool_val> FALSE
/* Nonterminal with return, which need to sepcify type */
%type <string> type
%type <string> id_expression
// %type <string> primary_expression

// For function def
%type <string> func_item // To get a single argument type


// For variable def
%type <string> expression
%type <string> assignment_expression
%type <string> logical_expression
%type <string> relational_expression
%type <string> unary_expression
%type <string> postfix_expression
%type <string> primary_expression
%type <string> constant

// For arithmetic
%type <string> additive_expression
%type <string> multi_expression
%type <string> arithmetic_operator
%type <string> assignment_operator
%type <string> cast_expression
%start program

/* Grammar section */
%%

program
    : program stat
    |
;

stat
    : declaration
    | compound_stat
    | expression_stat
    | print_func
    | selection_statement
    | iteration_statement
    | return_expression
;

declaration
    : type id_expression ASGN expression SEMICOLON {
        insert_symbol($1, $2, "variable", currentScope, false);
        int reg = -1;
        if (currentScope != 0) {
            reg = lookupRegForJasmin($2, currentScope);
        }
        outputVariableDef($2, $1, $4, currentScope, reg);
     }
    | type id_expression SEMICOLON {
        insert_symbol($1, $2, "variable", currentScope, false);
        int reg = -1;
        if (currentScope != 0) {
            reg = lookupRegForJasmin($2, currentScope);
        }
        outputVariableDef($2, $1, NULL, currentScope, reg);
    }
    | type id_expression LB func_item_list RB {
        insert_symbol($1, $2, "function", currentScope, false);
        char returnType[5] = {0};
        resolveType($1, returnType);
        outputFunctionDef($2, argumentsType, returnType);
    }
    | type id_expression LB func_item_list RB SEMICOLON {
        insert_symbol($1, $2, "function", currentScope, true);
    }
;

func_item_list
    : func_item_list COMMA func_item {
        // printf("In func_item_list of %s\n", $3);
        resolveType($3, argumentsType);
        // printf("argument %s\n", argumentsType);
    }
    | func_item {
        // printf("In func_item_list of last one %s\n", $1);
        resolveType($1, argumentsType);
    }
;

func_item
    : type id_expression {
        insert_symbol($1, $2, "parameter", currentScope + 1, false);
        // printf("In func_item\n");
        strcpy($$, $1);
    }
    |
;

compound_stat
    : LCB RCB
    | LCB { ++currentScope; } block_item_list RCB  { shouldDumpNow = true; --currentScope; }
;

block_item_list
    : block_item
    | block_item_list block_item
;

block_item
    : stat
;

expression_stat
    : SEMICOLON
    | expression SEMICOLON
;

print_func
    : PRINT LB QUOTA STR_CONST QUOTA RB SEMICOLON
    | PRINT LB primary_expression RB SEMICOLON
;

primary_expression
	: ID {
        lookup_symbol(yytext, false, false);
        if (isError) {
            makeError(false, false);
        }
    }
	| constant {
        strcpy($$, $1);
    }
	| LB expression RB {
        strcpy($$, $2);
    }
;

constant
    : F_CONST {
        char str[16] = {0};
        sprintf(str, "%f", $1);
        strcpy($$, str);
    }
    | I_CONST {
        char str[8] = {0};
        sprintf(str, "%d", $1);
        strcpy($$, str);
    }
    | QUOTA STR_CONST QUOTA {
        char str[64] = {0};
        sprintf(str, "%s", $2);
        strcpy($$, str);
    }
    | TRUE {
        char str[8] = {0};
        sprintf(str, "%s", "true");
        strcpy($$, str);
    }
    | FALSE {
        char str[8] = {0};
        sprintf(str, "%s", "false");
        strcpy($$, str);
    }
;

unary_operator
	: ADD
	| SUB
	| NOT
;

expression
	: assignment_expression {
        strcpy($$, $1);
    }
	| expression COMMA assignment_expression
;

return_expression
    : RET SEMICOLON
    | RET assignment_expression SEMICOLON
;

assignment_expression
	: unary_expression assignment_operator logical_expression {

    }
    | logical_expression {
        strcpy($$, $1);
    }
;

logical_expression
    : logical_expression OR relational_expression
    | logical_expression AND relational_expression
    | relational_expression {
        strcpy($$, $1);
    }
;

relational_expression
    : relational_expression relational_operator additive_expression
    | additive_expression {
        strcpy($$, $1);
    }
;

additive_expression
    : multi_expression
    | additive_expression ADD multi_expression {
        printf("In additive: %s %s\n", $1, $3);
    }
    | additive_expression SUB multi_expression
;

multi_expression
    : cast_expression
    | multi_expression MUL cast_expression {
        printf("%s%s\n", $1, $3);
        sprintf($$, "%s %s\n", $1, $3);
        // look up the table for this varaible
        int reg, scopeOfVariable, reg1, scopeOfVariable1;
        lookupVariable(&scopeOfVariable, &reg, $1, currentScope);
        lookupVariable(&scopeOfVariable1, &reg1, $3, currentScope);
    }
    | multi_expression DIV cast_expression
    | multi_expression MOD cast_expression
;

// arithmetic_expression
//     : arithmetic_expression arithmetic_operator unary_expression {
//         printf("arith: %s\n", $1);
//         printf("unary: %s\n", $3);
//         char s[32] = {0};
//         sprintf(s, "%s%s%s", $1, $2, $3);
//         printf("final %s\n", s);
//         // do smt
//     }
//     | unary_expression {
//         printf("in actual unary: %s\n", $1);
//         strcpy($$, $1);
//     }
// ;

relational_operator
    : MT
    | LT
    | MTE
    | LTE
    | EQ
    | NE
;

unary_expression
	: postfix_expression {
        strcpy($$, $1);
    }
	| INC unary_expression
	| DEC unary_expression
	| unary_operator cast_expression
;

cast_expression
    : unary_expression {
        strcpy($$, $1);
    }
    | LB type RB cast_expression
;

postfix_expression
	: primary_expression {
        strcpy($$, $1);
    }
	| postfix_expression LB RB {
        if (isError) {
            makeError(false, true);
        } 
    }
	| postfix_expression LB argument_expression_list RB  {
        if (isError) {
            makeError(false, true);
        } 
    }
	| postfix_expression INC
	| postfix_expression DEC
;

argument_expression_list
	: assignment_expression
	| argument_expression_list COMMA assignment_expression
;

// arithmetic_operator
//     : ADD {
//         strcpy($$, "+");
//     }
//     | SUB {
//         strcpy($$, "-");
//     }
//     | MUL {
//         strcpy($$, "*");
//     }
//     | DIV {
//         strcpy($$, "/");
//     }
//     | MOD {
//         strcpy($$, "%");
//     }
// ;

assignment_operator
	: ASGN {
        strcpy($$, "=");
    }
	| ADDASGN {
        strcpy($$, "+=");
    }
	| SUBASGN {
        strcpy($$, "-=");
    }
	| MULASGN {
        strcpy($$, "*=");
    }
	| DIVASGN {
        strcpy($$, "/=");
    }
	| MODASGN {
        strcpy($$, "%=");
    }
;

selection_statement
    : IF LB expression RB compound_stat ELSE compound_stat
    | IF LB expression RB compound_stat
;

iteration_statement
    : WHILE LB expression RB compound_stat
    | FOR LB declaration expression_stat expression RB
;

/* actions can be taken when meet the token or rule */
type
    : INT { strcpy($$, "int"); }
    | FLOAT { strcpy($$, "float"); }
    | BOOL { strcpy($$, "bool"); }
    | STRING { strcpy($$, "string"); }
    | VOID { strcpy($$, "void"); }
;

id_expression
    : ID { strcpy($$, yytext); }
;

%%

/* C code section */
int main(int argc, char** argv)
{
    char* fileTitle = ".class public compiler_hw3\n.super java/lang/Object\n";
    writeJasminFile(fileTitle);
    create_symbol();
    yylineno = 0;

    yyparse();
    dump_symbol(currentScope);
    TRAVERSE_TABLE {
        free(symbolTable[i]);
    }
	printf("\nTotal lines: %d \n",yylineno);
    return 0;
}

void yySemError(char* s) {
    memset(errorId, 0, sizeof(errorId));
    printf("\n|-----------------------------------------------|\n");
    printf("| Error found in line %d: %s\n", yylineno + 1, buf);
    printf("| %s", s);
    printf("\n|-----------------------------------------------|\n\n");
}

void yyerror(char *s)
{
    printf("%d:%s", yylineno + 1,buf);
    if (isError) {
        yySemError(errorMsg);
    }
    
    printf("\n|-----------------------------------------------|\n");
    printf("| Error found in line %d: %s\n", yylineno + 1, buf);
    printf("| %s", s);
    printf("\n|-----------------------------------------------|\n\n");
}

void create_symbol() {
    // init the table to null
    TRAVERSE_TABLE {
        symbolTable[i] = (variable_t*)malloc(sizeof(variable_t));
        symbolTable[i]->next = NULL;
    }
}

void insert_symbol(char* myType, char* name, char* entryType, int myScope, bool isForward) {
    // Create the new node
    variable_t* node = (variable_t*)malloc(sizeof(variable_t));
    // printf("currr: %s\n", name);
    // Init
    node->next = NULL;
    node->formalParameters = NULL;
    node->isDeclaredForward = isForward;
    node->scope = myScope;
    // Look up the reg
    if ((strcmp(entryType, "variable") == 0 || strcmp(entryType, "parameter") == 0) && myScope != 0) {
        node->reg = lookupRegForInsertion(myScope);
    } else {
        node->reg = -1;
    }
    printf("reg for: %d\n", node->reg);
    strcpy(node->name, name);
    strcpy(node->entryType, entryType);
    strcpy(node->myType,myType);
    // Append the list inside the function if it is a parameter
    if (strcmp(entryType, "parameter") == 0) {
        if (!parameters) {
            parameters = (char*)malloc(64);
            memset(parameters, 0, 64);
            strcpy(parameters, myType);
            // printf("1 parameters: %s\n", parameters);
        } else {
            char tmp[15] = {0};
            sprintf(tmp, "%s%s", ", ", myType);
            strcat(parameters, tmp);
            // printf("2 parameters: %s\n", parameters);
        } 
    }

    if (strcmp(entryType, "function") == 0) {
        // Is this a forward declaration function
        node->isDeclaredForward = isForward ? true : false;
        // printf("%s %d\n", name, node->isDeclaredForward);
        if (parameters) {
            node->formalParameters = (char*)malloc(64);
            // printf("parameters: %s\n", parameters);
            strcpy(node->formalParameters, parameters);
            free(parameters);
            parameters = NULL;
        }
    }

    if (strcmp(entryType, "function") == 0) {
        lookup_symbol(node->name, true, true);
    } else {
        // Check for redeclaration
        lookup_symbol(node->name, true, false);
    }
    
    if (isError) {
        if (strcmp("function", entryType) == 0) {
            makeError(true, true);
        } else {
            makeError(true, false);
        }
        free(node);
        return;
    }
    // Insert the node
    TRAVERSE_SINGLE_LIST(myScope) {
        if (!ptr->next) {
            ptr->next = node;
            break;
        }
    }
}

int lookupRegForInsertion(int scope) {
    int cur = 0;
    for (int i = 1; i <= scope; i++) {
        TRAVERSE_SINGLE_LIST_NEW(i) {
            if (cur < ptr->reg) {
                cur = ptr->reg;
            } else {
                cur++;
            }
        }
    }
    return cur;
}

int lookupRegForJasmin (char* name, int scope) {
    int cur = -1;
    TRAVERSE_SINGLE_LIST_NEW(scope) {
        if (strcmp(ptr->name, name) == 0) {
           cur = ptr->reg;
        }
    }
    return cur;
}

void lookupVariable (int* targetScope, int* targetReg, char* name, int scope) {
    for (int i = 0; i <= scope; i++) {
        TRAVERSE_SINGLE_LIST_NEW(scope) {
            if (strcmp(ptr->name, name) == 0) {
                *targetScope = ptr->scope;
                *targetReg = ptr->reg;
                return;
            }
        }
    }
    // Not finding anything, it means the target is a number not a variable 
    *targetScope = -1;
    *targetReg = -1;
}

void lookup_symbol(char* targetId, bool isRe, bool isFunction) {
    int myScope = currentScope;
    if (isRe) {
        // if (isFunction) printf("looking forrrrrr %s in scope: %d\n", targetId, myScope);
        TRAVERSE_SINGLE_LIST(myScope) {
            if (strcmp(ptr->name, targetId) == 0) {
                // printf("%s****%d\n", ptr->name, ptr->isDeclaredForward);
                // Redeclaraed
                // Error flag
                if (isFunction) {
                    if (ptr->isDeclaredForward) {
                        continue;
                    } else {
                        isError = true;
                    }
                } else { // variable, set isError to true directly
                    isError = true;
                }
                strcpy(errorId, targetId);
                return;
            }
        }
    } else {
        while(myScope >= 0) {
            TRAVERSE_SINGLE_LIST(myScope) {
                // Check undeclared variable/function
                if (strcmp(ptr->name, targetId) == 0) {
                    return;
                }
            }
            myScope--;
        }
        // Error flag
        isError = true;
        // Get the id name
        strcpy(errorId, targetId);
        // printf("%s\n", errorId);
    }
}
void dump_symbol(int currentScope) {
    // No variables in this scope
    if (!symbolTable[currentScope]->next) return;
    printf("\n%-10s%-10s%-12s%-10s%-10s%-10s\n",
        "Index", "Name", "Kind", "Type", "Scope", "Attribute");
    int index = 0;
    TRAVERSE_SINGLE_LIST(currentScope) {
        if (ptr == symbolTable[currentScope]) continue;
        if (ptr->isDeclaredForward) continue;
        printf("\n%-10d%-10s%-12s%-10s%-10d%-10s",
            index++, ptr->name, ptr->entryType, ptr->myType, ptr->scope,
            ptr->formalParameters ? ptr->formalParameters : "");
    }
    printf("\n\n");
    variable_t* cur = symbolTable[currentScope]->next, *prev = NULL;
    // Dump the list
    while(cur) {
        prev = cur;
        cur = cur->next;
        // Free the formalList of functions
        if (prev->formalParameters) {
            free(prev->formalParameters);
        }
        free(prev);
    }
    symbolTable[currentScope]->next = NULL;
}

void makeError(bool isRe, bool isFunction) {
    // Clear the buffer
    memset(errorMsg, 0, sizeof(errorMsg));
    if (isRe) {
        if (isFunction) {
            sprintf(errorMsg, "%s%s%s", "Redeclared", " function ", errorId);
        } else {
            sprintf(errorMsg, "%s%s%s", "Redeclared", " variable ", errorId);
        }
    } else {
        if (isFunction) {
            sprintf(errorMsg, "%s%s%s", "Undeclared", " function ", errorId);
        } else {
            sprintf(errorMsg, "%s%s%s", "Undeclared", " variable ", errorId);
        }
    }
}
