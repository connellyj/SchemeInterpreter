#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "interpreter.h"

void interpret(Value *tree) {
    assert(tree->type == CONS_TYPE);
    Value *cur = tree;
    while(!isNull(cur)) {
        Frame *frame = (Frame *)talloc(sizeof(Frame));
        display(eval(car(cur), frame));
        printf("\n");
        cur = cdr(cur);
    }
}

Value *eval(Value *expr, Frame *frame) {
    if(expr->type == INT_TYPE || expr->type == DOUBLE_TYPE ||
        expr->type == BOOL_TYPE || expr->type == STR_TYPE) {
        return expr;
    } else if(expr->type == SYMBOL_TYPE) {

    } else if(expr->type == CONS_TYPE) {

    }
    return makeNull();
}
