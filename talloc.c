#include <stdlib.h>
#include <stdbool.h>
#include "value.h"

Value *head;
bool initialized = false;

// Creates a new CONS_TYPE value node where the car is a PTR_TYPE Value and the cdr is null
Value *initPointerValue(void *p) {
    Value *pointerValue = (Value *)malloc(sizeof(Value));
    pointerValue->type = PTR_TYPE;
    pointerValue->p = p;

    Value *consValue = (Value *)malloc(sizeof(Value));
    consValue->type = CONS_TYPE;
    consValue->c.car = pointerValue;
    consValue->c.cdr = NULL;
    return consValue;
}

// Mallocs space of the given size and tracks the pointer in a linked list
void *talloc(size_t size) {
    void *p = malloc(size);
    Value *n = initPointerValue(p);
    if(!initialized) {
        head = n;
        initialized = true;
    } else {
        Value *cur = head;
        while(cur->c.cdr != NULL) {
            cur = cur->c.cdr;
        }
        cur->c.cdr = n;
    }
    return p;
}

// Frees all the pointers tracked in the linked list as well as the linked list itself
void tfree() {
    Value *cur = head;
    Value *next;
    while(cur != NULL) {
        next = cur->c.cdr;
        free(cur->c.car->p);
        free(cur->c.car);
        free(cur);
        cur = next;
    }
    initialized = false;
}

// Frees all memory and then exits the program
void texit(int status) {
    tfree();
    exit(status);
}
