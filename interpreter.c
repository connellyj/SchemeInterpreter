#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "interpreter.h"

void evalError(int errorCode) {
    if(errorCode == 1) printf("\'if\' requires 3 arguments");
    if(errorCode == 2) printf("the first argument of \'if\' must evaluate to a boolean");
    if(errorCode == 3) printf("First token in a list must be a symbol");
    if(errorCode == 4) printf("Symbol does not exist");
    if(errorCode == 5) printf("\'let\' requires a list of tuples as the first argument");
    printf("\n");
    texit(errorCode);
}

Value *var(Value *binding) {
    assert(binding->type == BINDING_TYPE);
    return binding->b.var;
}

Value *val(Value *binding) {
    assert(binding->type == BINDING_TYPE);
    return binding->b.val;
}

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

Value *makeBinding(Value *var, Value *val) {
    Value *newBinding = (Value *)talloc(sizeof(Value));
    newBinding->type = BINDING_TYPE;
    newBinding->b.var = var;
    newBinding->b.val = val;
    return newBinding;
}

Value *evalLet(Value *args, Frame *frame) {
    Value *bindings = car(args);
    Value *expr = car(cdr(args));
    Value *curBinding;
    Value *bindingsList = makeNull();
    while(!isNull(bindings)) {
        curBinding = car(bindings);
        Value *var = car(curBinding);
        Value *val = car(cdr(curBinding));
        bindingsList = cons(makeBinding(var, val), bindingsList);
        bindings = cdr(bindings);
    }
    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = bindingsList;
    return eval(expr, newFrame);


    // Value *assignments = car(args);
    // if(assignments->type != CONS_TYPE) evalError(5);
    // Value *curBinding = car(assignments);
    // Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    // Value *bindings = makeNull();
    // while(!isNull(curBinding)) {
    //     if(curBinding->type != CONS_TYPE) evalError(5);
    //     if(length(curBinding) != 2) evalError(5);
    //     Value *var = car(curBinding);
    //     Value *val = car(cdr(curBinding));
    //     bindings = cons(makeBinding(var, val), bindings);
    //     assignments = cdr(assignments);
    //     curBinding = car(assignments);
    // }
    // newFrame->bindings = bindings;
    // newFrame->parent = frame;
    // return eval(cdr(args), newFrame);
}

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

Value *eval(Value *expr, Frame *frame) {
    if(expr->type == INT_TYPE || expr->type == DOUBLE_TYPE ||
        expr->type == BOOL_TYPE || expr->type == STR_TYPE) {
        return expr;
    } else if(expr->type == SYMBOL_TYPE) {
        return lookupSymbol(expr, frame);
    } else if(expr->type == CONS_TYPE) {
        Value *first = car(expr);
        Value *args = cdr(expr);
        if(!strcmp(first->s, "if")) return evalIf(args, frame);
        if(!strcmp(first->s, "let")) return evalLet(args, frame);
        else evalError(4);
    }
    return makeNull();
}
