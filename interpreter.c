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
    else if(errorCode == 8) printf("\'quote\' requires one argument");
    else if(errorCode == 9) printf("\'define\' requires two arguments");
    else if(errorCode == 10) printf("\'define\' can only assign expressions to symbols");
    else if(errorCode == 11) printf("\'lambda\' requires two arguments");
    else if(errorCode == 12) printf("The first argument of \'lambda\' must be a list of arguments");
    else if(errorCode == 13) printf("All arguments to \'+\' must evaluate to numbers");
    else if(errorCode == 14) printf("Not enough arguments provided");
    else if(errorCode == 15) printf("Too many arguments provided");
    else if(errorCode == 16) printf("\'null?\' requires one argument");
    else if(errorCode == 17) printf("\'car\' requires one argument");
    else if(errorCode == 18) printf("\'cdr\' requires one argument");
    else if(errorCode == 19) printf("\'cons\' requires two arguments");
    else if(errorCode == 20) printf("\'car\' requires a list as an argument");
    else if(errorCode == 21) printf("\'cdr\' requires a list as an argument");
    else if(errorCode == 22) printf("\'zero?\' requires one argument");
    else if(errorCode == 23) printf("\'zero?\' requires a number as an argument");
    else if(errorCode == 24) printf("\'and\' requires 2 arguments");
    else if(errorCode == 25) printf("\'and\' requires booleans as arguments");
    else if(errorCode == 26) printf("\'or\' requires 2 arguments");
    else if(errorCode == 27) printf("\'or\' requires booleans as arguments");
    else if(errorCode == 28) printf("\'cond\' requires tuples where the first item evaluates to a boolean as arguments");
    else if(errorCode == 29) printf("\'/\' requires two numbers as arguments");
    else if(errorCode == 30) printf("Division by zero");
    else if(errorCode == 31) printf("All arguments to \'*\' must evaluate to numbers");
    else if(errorCode == 32) printf("\'modulo\' requires two integer arguments");
    else if(errorCode == 33) printf("\'<\' requires two numerical arguments");
    else if(errorCode == 34) printf("\'>\' requires two numerical arguments");
    else if(errorCode == 35) printf("\'=\' requires two numerical arguments");
    else if(errorCode == 36) printf("\'<=\' requires two numerical arguments");
    else if(errorCode == 37) printf("\'>=\' requires two numerical arguments");
    else printf("Evaluation error");
    printf("\n");
    texit(errorCode);
}

// Evaluates an if expression
// Causes an evaluation error if there are not two arguments
Value *evalIf(Value *args, Frame *frame) {
    // error checks
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
// Causes an evaluation error if there's not two arguments,
//      or if the first parameter is not a list of tuples where
//      the first value in each tuple is a valid variable name
Value *evalLet(Value *args, Frame *frame) {
    // error checking
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

// Evaluates a let* expression (like let, but evaluates left to right and
//      allows for linear dependency in the parameters)
// Causes an evaluation error if there's not two arguments,
//      or if the first parameter is not a list of tuples where
//      the first value in each tuple is a valid variable name
Value *evalLetStar(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(6);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(6);

    Value *bindings = car(args);
    Value *expr = car(cdr(args));
    Value *curBinding;
    Value *bindingsList = makeNull();
    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame;
    while(!isNull(bindings)) {
        if(bindings->type != CONS_TYPE) evalError(5);
        curBinding = car(bindings);
        if(curBinding->type != CONS_TYPE) evalError(5);
        if(length(curBinding) != 2) evalError(5);
        Value *var = car(curBinding);
        if(var->type != SYMBOL_TYPE) evalError(2);
        Value *val = eval(car(cdr(curBinding)), newFrame);
        bindingsList = cons(makeBinding(var, val), bindingsList);
        newFrame->bindings = bindingsList;
        bindings = cdr(bindings);
    }
    return eval(expr, newFrame);
}

// Evaluates a letrec expression
// Causes an evaluation error if there's not two arguments,
//      or if the first parameter is not a list of tuples where
//      the first value in each tuple is a valid variable name
Value *evalLetRec(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(6);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(6);

    Value *bindings = car(args);
    Value *expr = car(cdr(args));
    Value *curBinding;
    Value *bindingsList = makeNull();
    Value *values = makeNull();
    while(!isNull(bindings)) {
        if(bindings->type != CONS_TYPE) evalError(5);
        curBinding = car(bindings);
        if(curBinding->type != CONS_TYPE) evalError(5);
        if(length(curBinding) != 2) evalError(5);
        Value *var = car(curBinding);
        if(var->type != SYMBOL_TYPE) evalError(2);
        values = cons(car(cdr(curBinding)), values);
        Value *val = (Value *)talloc(sizeof(Value));
        val->type = BOOL_TYPE;
        val->i = false;
        bindingsList = cons(makeBinding(var, val), bindingsList);
        bindings = cdr(bindings);
    }
    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = bindingsList;
    curBinding = newFrame->bindings;
    while(!isNull(values)) {
        car(curBinding)->b.val = eval(car(values), newFrame);
        curBinding = cdr(curBinding);
        values = cdr(values);
    }
    return eval(expr, newFrame);
}

// Evaluates a quote expression
// Causes an evaluation error if there's not one argument
Value *evalQuote(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(8);
    assert(args->type == CONS_TYPE);
    if(length(args) != 1) evalError(8);

    return car(args);
}

// Evaluates a define expression
// Causes an evaluation error if there's not two arguments,
//      or if the first argument is not a valid variable name
Value *evalDefine(Value *args, Frame *frame) {
    // error checking
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

// Looks up the given symbol in the given frame and its parents and changes
//      its value to the given new value
// Throws an evaluation if the symbol doesn't exist
void changeSymbol(Value *symbol, Value *value, Frame *frame) {
    // error checking
    assert(symbol);
    assert(frame);
    assert(symbol->type == SYMBOL_TYPE);

    Frame *curFrame = frame;
    while(curFrame != NULL) {
        Value *curBinding = curFrame->bindings;
        while(!isNull(curBinding)) {
            if(!strcmp(symbol->s, var(car(curBinding))->s)) {
                car(curBinding)->b.val = value;
                return;
            }
            curBinding = cdr(curBinding);
        }
        curFrame = curFrame->parent;
    }
    evalError(4);
}

// Evaluates a set! expression
// Causes an evaluation error if there's not two arguments,
//      or if the first argument is not a valid variable name
Value *evalSet(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(9);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(9);

    Value *var = car(args);
    if(var->type != SYMBOL_TYPE) evalError(10);
    Value *val = eval(car(cdr(args)), frame);
    changeSymbol(var, val, frame);
    return makeVoid();
}

// Evaluates a lambda expression
// Causes an evaluation error if there's not two arguments,
//      or if the second argument is not a list of parameters
Value *evalLambda(Value *args, Frame *frame) {
    // error checking
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

// Evaluates a begin expression
Value *evalBegin(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) return makeVoid();
    assert(args->type == CONS_TYPE);

    Value *cur = args;
    while(!isNull(cdr(cur))) {
        eval(car(cur), frame);
        cur = cdr(cur);
    }
    return eval(car(cur), frame);
}

// Evaluates a cond expression
// Causes an evaluation error if the arguments are not lists
//      of length 2, or if the first argument of those lists
//      is not a boolean (or else or #t for the last argument)
Value *evalCond(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) return makeVoid();
    assert(args->type == CONS_TYPE);

    Value *cur = args;
    Value *cond;
    while(!isNull(cdr(cur))) {
        if(car(cur)->type != CONS_TYPE) evalError(28);
        if(length(car(cur)) != 2) evalError(28);
        cond = eval(car(car(cur)), frame);
        if(cond->type != BOOL_TYPE) evalError(28);
        if(cond->i) return eval(car(cdr(car(cur))), frame);
        cur = cdr(cur);
    }
    if(car(cur)->type == BOOL_TYPE && car(cur)->i) return car(cur);
    if(car(cur)->type != CONS_TYPE) evalError(28);
    if(length(car(cur)) != 2) evalError(28);
    if(!strcmp(car(car(cur))->s, "else")) return eval(car(cdr(car(cur))), frame);
    cond = eval(car(car(cur)), frame);
    if(cond->type != BOOL_TYPE) evalError(28);
    if(cond->i) return eval(car(cdr(car(cur))), frame);
    else return makeVoid();
}

// Evaluates an and expression
// Causes an evaluation error if there's not two arguments,
//      or if the arguments aren't booleans
Value *evalAnd(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(24);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(24);

    Value *cur = args;
    Value *cond;
    while(!isNull(cur)) {
        cond = eval(car(cur), frame);
        if(cond->type != BOOL_TYPE) evalError(25);
        if(!(cond->i)) return cond;
        cur = cdr(cur);
    }
    return cond;
}

// Evaluates an or expression
// Causes an evaluation error if there's not two arguments,
//      or if the arguments aren't booleans
Value *evalOr(Value *args, Frame *frame) {
    // error checking
    assert(args);
    assert(frame);
    if(isNull(args)) evalError(26);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(26);

    Value *cur = args;
    Value *cond;
    while(!isNull(cur)) {
        cond = eval(car(cur), frame);
        if(cond->type != BOOL_TYPE) evalError(27);
        if(cond->i) return cond;
        cur = cdr(cur);
    }
    return cond;
}

// Evaluates a + expression
// Causes an evaluation error if any of the arguments are not numbers
Value *primitiveAdd(Value *args) {
    // error checking
    assert(args);
    assert(args->type == CONS_TYPE || isNull(args));

    Value *result = (Value *)talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    result->d = 0;
    if(length(args) == 0) return result;
    Value *cur = args;
    while(!isNull(cur)) {
        if(car(cur)->type == INT_TYPE) result->d += (car(cur))->i;
        else if(car(cur)->type == DOUBLE_TYPE) result->d += (car(cur))->d;
        else evalError(13);
        cur = cdr(cur);
    }
    return result;
}

// Evaluates a * expression
// Causes an evaluation error if any of the arguments are not numbers
Value *primitiveMultiply(Value *args) {
    // error checking
    assert(args);
    assert(args->type == CONS_TYPE || isNull(args));

    Value *result = (Value *)talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    result->d = 0;
    if(length(args) == 0) return result;
    result->d = 1;
    Value *cur = args;
    while(!isNull(cur)) {
        if(car(cur)->type == INT_TYPE) result->d *= (car(cur))->i;
        else if(car(cur)->type == DOUBLE_TYPE) result->d *= (car(cur))->d;
        else evalError(31);
        cur = cdr(cur);
    }
    return result;
}

// Evaluates a / expression
// Causes an evaluation error if there aren't two arguments,
//      if either argument is not a number,
//      or if the second argument is a zero
Value *primitiveDivide(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(29);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(29);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if((n1->type != DOUBLE_TYPE && n1->type != INT_TYPE) ||
       (n2->type != DOUBLE_TYPE && n2->type != INT_TYPE)) evalError(29);
    double val1;
    double val2;
    if(n1->type == DOUBLE_TYPE) val1 = n1->d;
    else val1 = n1->i;
    if(n2->type == DOUBLE_TYPE) val2 = n2->d;
    else val2 = n2->i;
    if(val2 == 0) evalError(30);
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = DOUBLE_TYPE;
    res->d = val1 / val2;
    return res;
}

// Evaluates a modulo expression
// Causes an evaluation error if there aren't two arguments,
//      if either argument is not a number
Value *primitiveModulo(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(32);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(32);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if(n1->type != INT_TYPE || n2->type != INT_TYPE) evalError(32);
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = INT_TYPE;
    res->i = n1->i % n2->i;
    return res;
}

// Evaluates a - expression
// Causes an evaluation error if any of the arguments are not numbers
Value *primitiveSubtract(Value *args) {
    // error checking
    assert(args);
    assert(args->type == CONS_TYPE || isNull(args));

    Value *result = (Value *)talloc(sizeof(Value));
    result->type = DOUBLE_TYPE;
    result->d = 0;
    if(length(args) == 0) return result;
    if(car(args)->type == INT_TYPE) result->d = car(args)->i;
    else if(car(args)->type == DOUBLE_TYPE) result->d = car(args)->d;
    else evalError(13);
    Value *cur = cdr(args);
    while(!isNull(cur)) {
        if(car(cur)->type == INT_TYPE) result->d -= (car(cur))->i;
        else if(car(cur)->type == DOUBLE_TYPE) result->d -= (car(cur))->d;
        else evalError(13);
        cur = cdr(cur);
    }
    return result;
}

// Evaluates a < expression
// Causes an evaluation error if there aren't two numerical arguments
Value *primitiveLessThan(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(33);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(33);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if((n1->type != DOUBLE_TYPE && n1->type != INT_TYPE) ||
       (n2->type != DOUBLE_TYPE && n2->type != INT_TYPE)) evalError(33);
    double val1;
    double val2;
    if(n1->type == DOUBLE_TYPE) val1 = n1->d;
    else val1 = n1->i;
    if(n2->type == DOUBLE_TYPE) val2 = n2->d;
    else val2 = n2->i;
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = BOOL_TYPE;
    res->i = val1 < val2;
    return res;
}

// Evaluates a > expression
// Causes an evaluation error if there aren't two numerical arguments
Value *primitiveGreaterThan(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(34);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(34);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if((n1->type != DOUBLE_TYPE && n1->type != INT_TYPE) ||
       (n2->type != DOUBLE_TYPE && n2->type != INT_TYPE)) evalError(34);
    double val1;
    double val2;
    if(n1->type == DOUBLE_TYPE) val1 = n1->d;
    else val1 = n1->i;
    if(n2->type == DOUBLE_TYPE) val2 = n2->d;
    else val2 = n2->i;
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = BOOL_TYPE;
    res->i = val1 > val2;
    return res;
}

// Evaluates a = expression
// Causes an evaluation error if there aren't two numerical arguments
Value *primitiveEqualTo(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(35);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(35);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if((n1->type != DOUBLE_TYPE && n1->type != INT_TYPE) ||
       (n2->type != DOUBLE_TYPE && n2->type != INT_TYPE)) evalError(35);
    double val1;
    double val2;
    if(n1->type == DOUBLE_TYPE) val1 = n1->d;
    else val1 = n1->i;
    if(n2->type == DOUBLE_TYPE) val2 = n2->d;
    else val2 = n2->i;
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = BOOL_TYPE;
    res->i = val1 == val2;
    return res;
}

// Evaluates a <= expression
// Causes an evaluation error if there aren't two numerical arguments
Value *primitiveLessThanOrEqualTo(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(36);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(36);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if((n1->type != DOUBLE_TYPE && n1->type != INT_TYPE) ||
       (n2->type != DOUBLE_TYPE && n2->type != INT_TYPE)) evalError(36);
    double val1;
    double val2;
    if(n1->type == DOUBLE_TYPE) val1 = n1->d;
    else val1 = n1->i;
    if(n2->type == DOUBLE_TYPE) val2 = n2->d;
    else val2 = n2->i;
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = BOOL_TYPE;
    res->i = val1 <= val2;
    return res;
}

// Evaluates a >= expression
// Causes an evaluation error if there aren't two numerical arguments
Value *primitiveGreaterThanOrEqualTo(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(37);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(37);

    Value *n1 = car(args);
    Value *n2 = car(cdr(args));
    if((n1->type != DOUBLE_TYPE && n1->type != INT_TYPE) ||
       (n2->type != DOUBLE_TYPE && n2->type != INT_TYPE)) evalError(37);
    double val1;
    double val2;
    if(n1->type == DOUBLE_TYPE) val1 = n1->d;
    else val1 = n1->i;
    if(n2->type == DOUBLE_TYPE) val2 = n2->d;
    else val2 = n2->i;
    Value *res = (Value *)talloc(sizeof(Value));
    res->type = BOOL_TYPE;
    res->i = val1 >= val2;
    return res;
}

// Evaluates a null? expression
// Causes an evaluation error if there's not one argument
Value *primitiveIsNull(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(16);
    assert(args->type == CONS_TYPE);
    if(length(args) != 1) evalError(16);

    Value *boolVal = (Value *)talloc(sizeof(Value));
    boolVal->type = BOOL_TYPE;
    boolVal->i = isNull(car(args));
    return boolVal;
}

// Evaluates a zero? expression
// Causes an evaluation error if there's not one argument or if the argument
//      isn't a number
Value *primitiveIsZero(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(22);
    assert(args->type == CONS_TYPE);
    if(length(args) != 1) evalError(22);

    Value *boolVal = (Value *)talloc(sizeof(Value));
    boolVal->type = BOOL_TYPE;
    if(car(args)->type == INT_TYPE) boolVal->i = car(args)->i == 0;
    else if(car(args)->type == DOUBLE_TYPE) boolVal->i = car(args)->d == 0;
    else evalError(23);
    return boolVal;
}

// Evaluates a car expression
// Causes an evaluation error if there's not one argument,
//      or if the argument is not a list
Value *primitiveCar(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(17);
    assert(args->type == CONS_TYPE);
    if(length(args) != 1) evalError(17);
    if(car(args)->type != CONS_TYPE) evalError(20);

    return car(car(args));
}

// Evaluates a cdr expression
// Causes an evaluation error if there's not one argument,
//      or if the argument is not a list
Value *primitiveCdr(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(18);
    assert(args->type == CONS_TYPE);
    if(length(args) != 1) evalError(18);
    if(car(args)->type != CONS_TYPE) evalError(21);

    return cdr(car(args));
}

// Evaluates a cons expression
// Causes an evaluation error if there's not two arguments
Value *primitiveCons(Value *args) {
    // error checking
    assert(args);
    if(isNull(args)) evalError(19);
    assert(args->type == CONS_TYPE);
    if(length(args) != 2) evalError(19);

    return cons(car(args), car(cdr(args)));
}

// Binds the given function to the given name in the given frame
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // error checking
    assert(name);
    assert(function);
    assert(frame);

    Value *value = (Value *)talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *symbol = (Value *)talloc(sizeof(Value));
    symbol->type = SYMBOL_TYPE;
    symbol->s = name;
    Value *binding = makeBinding(symbol, value);
    frame->bindings = cons(binding, frame->bindings);
}

// Looks up the given symbol in the given frame and its parents
// Throws an evaluation if the symbol doesn't exist
Value *lookupSymbol(Value *symbol, Frame *frame) {
    // error checking
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
        curFrame = curFrame->parent;
    }
    evalError(4);
    return makeNull();
}

// Helper function to copy a frame completely (deep clone)
Frame *copyFrame(Frame *frame) {
    // error checking
    assert(frame);

    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame->parent;
    newFrame->bindings = frame->bindings;
    return newFrame;
}

// Helper function that applies a closure to the given arguments
// Causes an evaluation error if there are not enough or too many
//      arguments for the given function
Value *applyClosure(Value *function, Value *args) {
    // error checking
    assert(function);
    assert(args);
    assert(function->type == CLOSURE_TYPE);
    assert(args->type == CONS_TYPE || isNull(args));

    Frame *frame = copyFrame(function->cl.frame);
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

// Applys a function that is a primitve function to the given arguments
Value *applyPrimitive(Value *function, Value *args) {
    // error checking
    assert(function);
    assert(args);
    assert(function->type == PRIMITIVE_TYPE);
    assert(args->type == CONS_TYPE || isNull(args));

    return (function->pf)(args);
}

// Executes the given function using the given arguments
Value *apply(Value *function, Value *args) {
    // error checking
    assert(function);
    assert(args);
    assert(args->type == CONS_TYPE || isNull(args));
    assert(function->type == CLOSURE_TYPE || function->type == PRIMITIVE_TYPE);

    if(function->type == CLOSURE_TYPE) return applyClosure(function, args);
    else return applyPrimitive(function, args);
}

// Returns a list of each argument evaluated
Value *evalEach(Value *args, Frame *frame) {
    // error checking
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
// Throws an evaluation error if an invalid function is called,
//      or if an unexpected error occurs
Value *eval(Value *expr, Frame *frame) {
    // error checking
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

        // special forms
        if(!strcmp(first->s, "if")) return evalIf(args, frame);
        if(!strcmp(first->s, "cond")) return evalCond(args, frame);
        if(!strcmp(first->s, "and")) return evalAnd(args, frame);
        if(!strcmp(first->s, "or")) return evalOr(args, frame);
        if(!strcmp(first->s, "let")) return evalLet(args, frame);
        if(!strcmp(first->s, "let*")) return evalLetStar(args, frame);
        if(!strcmp(first->s, "letrec")) return evalLetRec(args, frame);
        if(!strcmp(first->s, "quote")) return evalQuote(args);
        if(!strcmp(first->s, "define")) return evalDefine(args, frame);
        if(!strcmp(first->s, "set!")) return evalSet(args, frame);
        if(!strcmp(first->s, "lambda")) return evalLambda(args, frame);
        if(!strcmp(first->s, "begin")) return evalBegin(args, frame);

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

// Interprets the given parsed scheme program
void interpret(Value *tree) {
    // error checking
    assert(tree);
    assert(tree->type == CONS_TYPE);

    // binds primitive functions to the top level frame
    Frame *frame = (Frame *)talloc(sizeof(Frame));
    frame->parent = NULL;
    frame->bindings = makeNull();
    bind("+", primitiveAdd, frame);
    bind("-", primitiveSubtract, frame);
    bind("null?", primitiveIsNull, frame);
    bind("zero?", primitiveIsZero, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
    bind("*", primitiveMultiply, frame);
    bind("/", primitiveDivide, frame);
    bind("modulo", primitiveModulo, frame);
    bind("<", primitiveLessThan, frame);
    bind(">", primitiveGreaterThan, frame);
    bind("=", primitiveEqualTo, frame);
    bind("<=", primitiveLessThanOrEqualTo, frame);
    bind(">=", primitiveGreaterThanOrEqualTo, frame);

    Value *cur = tree;
    Value *evaled;
    while(!isNull(cur)) {
        evaled = eval(car(cur), frame);
        display(evaled);
        if(evaled->type != VOID_TYPE) printf("\n");
        cur = cdr(cur);
    }
}
