#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "talloc.h"

// Helper function to get the car of a "cons cell"
Value *car(Value *list) {
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    return list->c.car;
}

// Helper function to get the cdr of a "cons cell"
Value *cdr(Value *list) {
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Helper function to set the car of a "cons cell"
void setCar(Value *list, Value *newCar) {
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    list->c.car = newCar;
}

// Helper function to set the cdr of a "cons cell"
void setCdr(Value *list, Value *newCdr) {
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    list->c.cdr = newCdr;
}

// Helper function to check if the given value is a null value node
bool isNull(Value *value) {
    assert(value != NULL);
    if(value->type == NULL_TYPE) return true;
    return false;
}

// Returns the length of the list
int length(Value *value) {
    assert(value != NULL);
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

// Create a new CONS_TYPE value node
Value *cons(Value *car, Value *cdr) {
    Value *consValue = (Value *)talloc(sizeof(Value));
    consValue->type = CONS_TYPE;
    setCar(consValue, car);
    setCdr(consValue, cdr);
    return consValue;
}

// Helper function to display a list of value nodes
void displayList(Value *list, bool addSpace) {
	if(list->type != CONS_TYPE && list->type != NULL_TYPE) {
		if(list->type == INT_TYPE) printf("%i", list->i);
    	else if (list->type == DOUBLE_TYPE) printf("%f", list->d);
    	else if (list->type == STR_TYPE || list->type == OPEN_TYPE ||
        	list->type == CLOSE_TYPE || list->type == SYMBOL_TYPE) {
        	printf("%s", list->s);
    	}
    	else if(list->type == BOOL_TYPE) {
        	if(list->i) printf("#t");
        	else printf("#f");
    	}
    	if(addSpace) printf(" ");
	}
    else if (list->type == CONS_TYPE) {
        bool print = (car(list))->type == CONS_TYPE;
        bool space = !isNull(cdr(list));
        if(print) printf("(");
        displayList(car(list), space);
        if(print) {
            if(space) printf(") ");
            else printf(")");
        }
        displayList(cdr(list), true);
    }
}

// Displays the given list on one line with parentheses denoting lists
void display(Value * list) {
    if(list->type == CONS_TYPE) {
        bool space = !isNull(cdr(list));
        printf("(");
        displayList(car(list), space);
        displayList(cdr(list), space);
        printf(") ");
    }else displayList(list, true);
}

// Helper method to copy a CONS_TYPE Value node
Value *copyConsValue(Value *val) {
    Value *copy = talloc(sizeof(Value));
    copy->type = val->type;
    if(copy->type == CONS_TYPE) {
        setCar(copy, car(val));
        setCdr(copy, cdr(val));
    }
    return copy;
}

// Reverses the given list
Value *reverse(Value *list) {
    assert(list != NULL);
    if(isNull(list)) return list;
    Value *cur = copyConsValue(list);
    Value *next = copyConsValue(cdr(cur));
    Value *prev;
    setCdr(cur, makeNull());
    prev = cur;
    cur = next;
    while(!isNull(cur)) {
        next = copyConsValue(cdr(cur));
        setCdr(cur, prev);
        prev = cur;
        cur = next;
    }
    return prev;
}