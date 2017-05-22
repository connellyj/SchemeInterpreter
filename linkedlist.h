#include <stdbool.h>
#include "value.h"

#ifndef _LINKEDLIST
#define _LINKEDLIST

// Create a new NULL_TYPE value node.
Value *makeNull();

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr);

// Display the contents of the linked list to the screen in some kind of readable format
void display(Value *list);

// Helper function to display a list of value nodes
void displayList(Value *list, bool addSpace);

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list);

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list);

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list);

// Helper function to set the cdr of a "cons cell"
void setCdr(Value *list, Value *newCdr);

// Helper function to set the car of a "cons cell"
void setCar(Value *list, Value *newCar);

// Helper function to get the variable of a binding
Value *var(Value *binding);

// Helper function to get the value of a binding
Value *val(Value *binding);

// Creates a BINDING_TYPE Value node
Value *makeBinding(Value *var, Value *val);

// Creates a VOID_TYPE Value node
Value *makeVoid();

// Creates a closure type Value node
Value *makeClosure(Value *paramNames, Value *functionCode, struct Frame *frame);

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value);

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value);


#endif
