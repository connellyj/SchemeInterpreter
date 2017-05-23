#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "interpreter.h"

// Helper function to print error codes and exit the program
void evalError(int errorCode) {
    if(errorCode == 1) printf("\'if\' requires 3 arguments");
    else if(errorCode == 2) printf("\'let\' can only assign expressions to symbols");
    else if(errorCode == 3) printf("Function name must be a symbol");
    else if(errorCode == 4) printf("Symbol undefined");
    else if(errorCode == 5) printf("\'let\' requires a list of tuples as the first argument");
    else if(errorCode == 6) printf("\'let\' requires 2 arguments");
    else if(errorCode == 7) printf("Evaluation error");
    else if(errorCode == 8) printf("\'quote\' only takes one argument");
    else if(errorCode == 9) printf("\'define\' only takes two arguments");
    else if(errorCode == 10) printf("\'define\' can only assign expressions to symbols");
    else if(errorCode == 11) printf("\'lambda\' only takes two arguments");
    else if(errorCode == 12) printf("The first argument of \'lambda\' must be a list of arguments");
    else if(errorCode == 13) printf("");
    else if(errorCode == 14) printf("Not enough arguments provided");
    else if(errorCode == 15) printf("Too many arguments provided");
    else printf("Evaluation error");
    printf("\n");
    texit(errorCode);
}

// Interprets the given parsed scheme program
void interpret(Value *tree) {
    assert(tree);
    assert(tree->type == CONS_TYPE);
    Value *cur = tree;
    Frame *frame = (Frame *)talloc(sizeof(Frame));
    frame->parent = NULL;
    frame->bindings = makeNull();
    Value *evaled;
    while(!isNull(cur)) {
        evaled = eval(car(cur), frame);
        display(evaled);
        if(evaled->type != VOID_TYPE) printf("\n");
        cur = cdr(cur);
    }
}

// Evaluates an if expression
Value *evalIf(Value *args, Frame *frame) {
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(1);
    assert(args->type == CONS_TYPE);
    if(length(args) != 3) evalError(1);
    Value *cond = car(args);
    Value *ifTrue = car(cdr(args));
    Value *ifFalse = car(cdr(cdr(args)));
    Value *result = eval(cond, frame);
    if(result->type != BOOL_TYPE || !(result->i)) return eval(ifFalse, frame);
    else return eval(ifTrue, frame);
}

// Evaluates a let expression
Value *evalLet(Value *args, Frame *frame) {
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(6);
    assert(args->type == CONS_TYPE);
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
        if(var->type != SYMBOL_TYPE) evalError(2);
        Value *val = eval(car(cdr(curBinding)), frame);
        bindingsList = cons(makeBinding(var, val), bindingsList);
        bindings = cdr(bindings);
    }
    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = bindingsList;
    return eval(expr, newFrame);
}

// Evaluates a quote expression
Value *evalQuote(Value *args) {
    assert(args);
    if(isNull(args)) evalError(8);
    assert(args->type == CONS_TYPE);
    if(length(args) != 1) evalError(8);
    return car(args);
}

// Evaluates a define expression
Value *evalDefine(Value *args, Frame *frame) {
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(9);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(9);
    Value *var = car(args);
    if(var->type != SYMBOL_TYPE) evalError(10);
    Value *val = eval(car(cdr(args)), frame);
    Value *binding = makeBinding(var, val);
    frame->bindings = cons(binding, frame->bindings);
    return makeVoid();
}

// Evaluates a lambda expression
Value *evalLambda(Value *args, Frame *frame) {
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(11);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(11);
    Value *params = car(args);
    if(params->type != CONS_TYPE && !isNull(params)) evalError(12);
    Value *code = car(cdr(args));
    return makeClosure(params, code, frame);
}

// Looks up the given symbol in the given frame and its parents
Value *lookupSymbol(Value *symbol, Frame *frame) {
    assert(symbol);
    assert(frame);
    assert(symbol->type == SYMBOL_TYPE);
    Frame *curFrame = frame;
    while(curFrame != NULL) {
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

// Executes the given function using the given arguments
Value *apply(Value *function, Value *args) {
    assert(function);
    assert(args);
    assert(args->type == CONS_TYPE || isNull(args));
    assert(function->type == CLOSURE_TYPE);
    Frame *frame = (Frame *)talloc(sizeof(Frame));
    frame = function->cl.frame;
    Value *values = args;
    Value *bindings = makeNull();
    Value *variables = function->cl.paramNames;
    while(!isNull(variables)) {
        if(isNull(values)) evalError(14);
        Value *var = car(variables);
        Value *val = car(values);
        Value *b = makeBinding(var, val);
        bindings = cons(b, bindings);
        values = cdr(values);
        variables = cdr(variables);
    }
    frame->bindings = bindings;
    if(!isNull(values)) evalError(15);
    return eval(function->cl.functionCode, frame);
}

// Returns a list of the evaluated args
Value *evalEach(Value *args, Frame *frame) {
    assert(args);
    assert(frame);
    assert(args->type == CONS_TYPE || isNull(args));
    Value *evaledArgs = makeNull();
    Value *cur = args;
    Value *evaled;
    while(!isNull(cur)) {
        evaled = eval(car(cur), frame);
        evaledArgs = cons(evaled, evaledArgs);
        cur = cdr(cur);
    }
    return reverse(evaledArgs);
}

// Evaluates the given scheme expression
Value *eval(Value *expr, Frame *frame) {
    assert(expr);
    assert(frame);
    if(expr->type == INT_TYPE || expr->type == DOUBLE_TYPE ||
        expr->type == BOOL_TYPE || expr->type == STR_TYPE ||
        expr->type == NULL_TYPE) {
        return expr;
    } else if(expr->type == SYMBOL_TYPE) {
        return lookupSymbol(expr, frame);
    } else if(expr->type == CONS_TYPE) {
        Value *first = car(expr);
        if(first->type != SYMBOL_TYPE && first->type != CONS_TYPE) evalError(3);
        Value *args = cdr(expr);
        if(!strcmp(first->s, "if")) return evalIf(args, frame);
        if(!strcmp(first->s, "let")) return evalLet(args, frame);
        if(!strcmp(first->s, "quote")) return evalQuote(args);
        if(!strcmp(first->s, "define")) return evalDefine(args, frame);
        if(!strcmp(first->s, "lambda")) return evalLambda(args, frame);
        else {
            Value *evaledOperator = eval(first, frame);
            Value *evaledArgs = evalEach(args, frame);
            return apply(evaledOperator, evaledArgs);
        }
    } else {
        evalError(7);
    }
    return makeNull();
}
