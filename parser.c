#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"

struct Stack {
    Value *top;
};
typedef struct Stack Stack;

// Helper function to initialize a stack
void initStack(Stack *stack) {
    stack->top = (Value *)talloc(sizeof(Value));
    stack->top->type = NULL_TYPE;
}

// Push the given item onto the given stack
void push(Stack *stack, Value *item) {
    Value *consValue = (Value *)talloc(sizeof(Value));
    consValue->type = CONS_TYPE;
    setCar(consValue, item);
    setCdr(consValue, stack->top);
    stack->top = consValue;
}

// Pop the next value off of the given stack
Value *pop(Stack *stack) {
    Value *popped = car(stack->top);
    stack->top = cdr(stack->top);
    return popped;
}

// Returns whether or not the given stack is empty
bool isEmpty(Stack *stack) {
    return isNull(stack->top);
}

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    Stack *stack = (Stack *)talloc(sizeof(Stack));
    initStack(stack);
    Value *curToken = tokens;
    int depth = 0;
    while(!isNull(curToken)) {
        // increase depth when there's an open paren
        if(car(curToken)->type == OPEN_TYPE) depth++;
        // close paren, so a rule has been completed
        if(car(curToken)->type == CLOSE_TYPE) {
            Value *cur = pop(stack);
            Value *list = makeNull();
            // pop everything from stack until next open paren
            // make list of popped items and push that onto the stack
            while(cur->type != OPEN_TYPE) {
                // if the stack is empty before another paren, throw error
                if(isEmpty(stack)) {
                    printf("Syntax error: too many close parentheses\n");
                    texit(2);
                }
                list = cons(cur, list);
                cur = pop(stack);
            }
            push(stack, list);
            depth--;
        }
        // otherwise, push item onto stack
        else {
            push(stack, car(curToken));
        }
        // move on to next token
        curToken = cdr(curToken);
    }
    // if depth is not zero, then there's a paren mismatch
    if(depth != 0) {
        printf("Syntax error: not enough close parentheses\n");
        texit(1);
    }
    // reverse the stack and return it
    Stack *final = (Stack *)talloc(sizeof(Stack));
    initStack(final);
    while(!isEmpty(stack)) push(final, pop(stack));
    return final->top;
}

// Prints the tree to the screen in a readable fashion,
// uses parentheses to indicate subtrees.
void printTree(Value *tree) {
    Value *cur = tree;
    while(!isNull(cur)) {
        display(car(cur));
        cur = cdr(cur);
    }
}
