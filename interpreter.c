#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "interpreter.h"

// Helper function to print error codes and exit the program
void evalError(int errorCode) {
    if(errorCode == 1) printf("\'if\' requires 3 arguments");
    else if(errorCode == 2) printf("The first argument of \'if\' must evaluate to a boolean");
    else if(errorCode == 3) printf("First token in a list must be a symbol");
    else if(errorCode == 4) printf("Symbol does not exist");
    else if(errorCode == 5) printf("\'let\' requires a list of tuples as the first argument");
    else if(errorCode == 6) printf("\'let\' requires 2 parameters");
    else if(errorCode == 7) printf("Evaluation error");
    else printf("Evaluation error");
    printf("\n");
    texit(errorCode);
}

// Helper function to get the variables of a binding
Value *var(Value *binding) {
    assert(binding->type == BINDING_TYPE);
    return binding->b.var;
}

// Helper function to get the value of a binding
Value *val(Value *binding) {
    assert(binding->type == BINDING_TYPE);
    return binding->b.val;
}

// Interprets the given parsed scheme program
void interpret(Value *tree) {
    assert(tree->type == CONS_TYPE);
    Value *cur = tree;
    while(!isNull(cur)) {
        Frame *frame = (Frame *)talloc(sizeof(Frame));
        frame->parent = NULL;
        display(eval(car(cur), frame));
        printf("\n");
        cur = cdr(cur);
    }
}

// Evaluates an if expression
Value *evalIf(Value *args, Frame *frame) {
    if(length(args) != 3) evalError(1);
    Value *cond = car(args);
    Value *ifTrue = car(cdr(args));
    Value *ifFalse = car(cdr(cdr(args)));
    Value *result = eval(cond, frame);
    if(result->type != BOOL_TYPE) evalError(2);
    if(result->i) return eval(ifTrue, frame);
    else return eval(ifFalse, frame);
}

// Creates a BINDING_TYPE Value node
Value *makeBinding(Value *var, Value *val) {
    Value *newBinding = (Value *)talloc(sizeof(Value));
    newBinding->type = BINDING_TYPE;
    newBinding->b.var = var;
    newBinding->b.val = val;
    return newBinding;
}

// Evaluates a let expression
Value *evalLet(Value *args, Frame *frame) {
    if(args->type != CONS_TYPE) evalError(5);
    if(length(args) != 2) evalError(6);
    Value *bindings = car(args);
    Value *expr = car(cdr(args));
    Value *curBinding;
    Value *bindingsList = makeNull();
    while(!isNull(bindings)) {
        if(bindings->type != CONS_TYPE) evalError(5);
        curBinding = car(bindings);
        if(curBinding->type != CONS_TYPE) evalError(5);
        if(length(curBinding) != 2) evalError(5);
        Value *var = car(curBinding);
        Value *val = eval(car(cdr(curBinding)), frame);
        bindingsList = cons(makeBinding(var, val), bindingsList);
        bindings = cdr(bindings);
    }
    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = bindingsList;
    return eval(expr, newFrame);
}

// Looks up the given symbol in the given frame and its parents
Value *lookupSymbol(Value *symbol, Frame *frame) {
    Frame *curFrame = frame;
    while(curFrame->parent != NULL) {
        Value *curBinding = curFrame->bindings;
        while(!isNull(curBinding)) {
            if(!strcmp(symbol->s, var(car(curBinding))->s)) return val(car(curBinding));
            curBinding = cdr(curBinding);
        }
        curFrame = frame->parent;
    }
    evalError(4);
    return makeNull();
}

// Evaluates the given scheme expression
Value *eval(Value *expr, Frame *frame) {
    if(expr->type == INT_TYPE || expr->type == DOUBLE_TYPE ||
        expr->type == BOOL_TYPE || expr->type == STR_TYPE) {
        return expr;
    } else if(expr->type == SYMBOL_TYPE) {
        return lookupSymbol(expr, frame);
    } else if(expr->type == CONS_TYPE) {
        Value *first = car(expr);
        if(first->type != SYMBOL_TYPE) evalError(3);
        Value *args = cdr(expr);
        if(!strcmp(first->s, "if")) return evalIf(args, frame);
        if(!strcmp(first->s, "let")) return evalLet(args, frame);
        else evalError(4);
    } else {
        evalError(7);
    }
    return makeNull();
}
