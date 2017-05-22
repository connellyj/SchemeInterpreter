#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "talloc.h"
#include "interpreter.h"
#include "linkedlist.h"

// Helper function to get the car of a "cons cell"
Value *car(Value *list) {
    assert(list);
    assert(list->type == CONS_TYPE);
    return list->c.car;
}

// Helper function to get the cdr of a "cons cell"
Value *cdr(Value *list) {
    assert(list);
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Helper function to get the variable of a binding
Value *var(Value *binding) {
    assert(binding);
    assert(binding->type == BINDING_TYPE);
    return binding->b.var;
}

// Helper function to get the value of a binding
Value *val(Value *binding) {
    assert(binding);
    assert(binding->type == BINDING_TYPE);
    return binding->b.val;
}

// Helper function to set the car of a "cons cell"
void setCar(Value *list, Value *newCar) {
    assert(list);
    assert(list->type == CONS_TYPE);
    list->c.car = newCar;
}

// Helper function to set the cdr of a "cons cell"
void setCdr(Value *list, Value *newCdr) {
    assert(list);
    assert(list->type == CONS_TYPE);
    list->c.cdr = newCdr;
}

// Helper function to check if the given value is a null value node
bool isNull(Value *value) {
    assert(value);
    if(value->type == NULL_TYPE) return true;
    return false;
}

// Returns the length of the list
int length(Value *value) {
    assert(value);
    Value *cur = value;
    int len = 0;
    while(!isNull(cur)) {
        assert(cur->type == CONS_TYPE);
        len++;
        cur = cdr(cur);
    }
    return len;
}

// Create a new NULL_TYPE value node
Value *makeNull() {
    Value *nullValue = (Value *)talloc(sizeof(Value));
    nullValue->type = NULL_TYPE;
    return nullValue;
}

// Creates a BINDING_TYPE Value node
Value *makeBinding(Value *var, Value *val) {
    assert(var);
    assert(val);
    Value *newBinding = (Value *)talloc(sizeof(Value));
    newBinding->type = BINDING_TYPE;
    newBinding->b.var = var;
    newBinding->b.val = val;
    return newBinding;
}

// Creates a VOID_TYPE Value node
Value *makeVoid() {
    Value *voidValue = (Value *)talloc(sizeof(Value));
    voidValue->type = VOID_TYPE;
    return voidValue;
}

// Creates a closure type Value node
Value *makeClosure(Value *paramNames, Value *functionCode, Frame *frame) {
    assert(paramNames);
    assert(functionCode);
    assert(frame);
    Value *closure = (Value *)talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl.paramNames = paramNames;
    closure->cl.functionCode = functionCode;
    Frame *newFrame = (Frame *)talloc(sizeof(Frame));
    newFrame->parent = frame;
    closure->cl.frame = newFrame;
    return closure;
}

// Create a new CONS_TYPE value node
Value *cons(Value *car, Value *cdr) {
    assert(car);
    assert(cdr);
    Value *consValue = (Value *)talloc(sizeof(Value));
    consValue->type = CONS_TYPE;
    setCar(consValue, car);
    setCdr(consValue, cdr);
    return consValue;
}

// Helper function to print a boolean
void displayBool(Value *boolVal) {
    assert(boolVal);
    assert(boolVal->type == BOOL_TYPE);
    if(boolVal->i) printf("#t");
    else printf("#f");
}

// Helper function to display a binding
void displayBinding(Value *binding) {
    assert(binding);
    assert(binding->type == BINDING_TYPE);
    printf("[");
    displayList(var(binding), false);
    printf(" = ");
    displayList(val(binding), false);
    printf("]");
}

// Helper function to display nested lists
void displayNestedList(Value *list) {
    assert(list);
    assert(list->type == CONS_TYPE);
    bool print = (car(list))->type == CONS_TYPE;
    bool space = !isNull(cdr(list));
    if(print) printf("(");
    displayList(car(list), space);
    if(print) {
        if(space) printf(") ");
        else printf(")");
    }
    if(space) displayList(cdr(list), false);
}

// Helper function to display a list of value nodes
void displayList(Value *list, bool addSpace) {
    assert(list);
    if(list->type == VOID_TYPE) return;
    if(list->type != CONS_TYPE) {
        if(list->type == INT_TYPE) printf("%i", list->i);
        else if (list->type == DOUBLE_TYPE) printf("%f", list->d);
        else if(list->type == NULL_TYPE) printf("()");
        else if(list->type == PTR_TYPE) printf("%p", list->p);
        else if(list->type == CLOSURE_TYPE) printf("closure");
        else if(list->type == BOOL_TYPE) displayBool(list);
        else if(list->type == BINDING_TYPE) displayBinding(list);
        else if (list->type == STR_TYPE || list->type == OPEN_TYPE ||
            list->type == CLOSE_TYPE || list->type == SYMBOL_TYPE) {
            printf("%s", list->s);
        }
        if(addSpace) printf(" ");
    }
    else displayNestedList(list);
}

// Displays the given list on one line with parentheses denoting lists
void display(Value *list) {
    assert(list);
    if(list->type == CONS_TYPE) {
        bool space = !isNull(cdr(list));
        printf("(");
        displayList(car(list), space);
        displayList(cdr(list), false);
        printf(") ");
    } else displayList(list, true);
}

// Helper method to copy a CONS_TYPE Value node
Value *copyConsValue(Value *val) {
    assert(val);
    assert(val->type == CONS_TYPE);
    Value *copy = (Value *)talloc(sizeof(Value));
    copy->type = val->type;
    setCar(copy, car(val));
    setCdr(copy, cdr(val));
    return copy;
}

// Reverses the given list
Value *reverse(Value *list) {
    assert(list);
    if(isNull(list)) return list;
    assert(list->type == CONS_TYPE);

    Value *cur = copyConsValue(list);
    Value *next = copyConsValue(cdr(cur));
    Value *prev;
    setCdr(cur, makeNull());
    prev = cur;
    cur = next;
    while(!isNull(cur)) {
        assert(cur->type == CONS_TYPE);
        next = copyConsValue(cdr(cur));
        setCdr(cur, prev);
        prev = cur;
        cur = next;
    }
    return prev;
}
