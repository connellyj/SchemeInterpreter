#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"

// Used to store information about a symbol, dynamically re-sizeable
typedef struct SymbolString SymbolString;
struct SymbolString {
    int size;
    int capacity;
    char *str;
};

// Initiailzes the given SymbolString with a capacity of 20
void initSymbolString(SymbolString *symbol) {
    assert(symbol);
    symbol->capacity = 20;
    symbol->size = 1;
    symbol->str = talloc(symbol->capacity * sizeof(char));
    symbol->str[0] = '\0';
}

// Doubles the size of the given symbol's string
void doubleArray(SymbolString *symbol) {
    assert(symbol);
    symbol->capacity *= 2;
    char *dubClone = talloc(symbol->capacity * sizeof(char));
    strcpy(dubClone, symbol->str);
    symbol->str = dubClone;
}

// Adds the given character to the end of the given symbol's string
void append(SymbolString *symbol, char c) {
    assert(symbol);
    if (symbol->size == symbol->capacity) {
        doubleArray(symbol);
    }
    symbol->str[symbol->size - 1] = c;
    symbol->str[symbol->size] = '\0';
    symbol->size++;
}

// Helper function to fill in a Value node with the given string
// Does not assign a type to the Value node
void makeStringMalloc(Value *val, char *str, int size) {
    assert(val);
    assert(str);
    char *p = talloc((size + 1) * sizeof(char));
    strcpy(p, str);
    val->s = p;
}

// Helper function to fill in a given Value node with the given integer
void makeInteger(Value *val, int num) {
    assert(val);
    val->type = INT_TYPE;
    val->i = num;
}

// Helper function to fill in a given Value node with the given float
void makeDouble(Value *val, double num) {
    assert(val);
    val->type = DOUBLE_TYPE;
    val->d = num;
}

// Helper function to fill in the given Value node with the given boolean
void makeBool(Value *val, bool b) {
    assert(val);
    val->type = BOOL_TYPE;
    val->i = (int)b;
}

// Helper function to fill in the given Value node with the given symbol
void makeSymbol(Value *val, char *str) {
    assert(val);
    assert(str);
    val->type = SYMBOL_TYPE;
    val->s = str;
}

// Helper function to fill in the given Value node with the given string
void makeString(Value *val, char *str) {
    assert(val);
    assert(str);
    val->type = STR_TYPE;
    val->s = str;
}

// Helper function to determine whether or not the given char could be part
// of a number
bool isNumber(char c) {
    return c == '0' || c == '1' || c == '2' ||
        c == '3' || c == '4' || c == '5' ||
        c == '6' || c == '7' || c == '8' ||
        c == '9' || c == '.';
}

// Helper function to determine whether the given char can be the first char
// in a symbol
bool isInitialSymbol(char c) {
    return c == '!' || c == '$' || c == '%' || c == '*' || c == '/' ||
        c == ':' || c == '<' || c == '=' || c == '>' || c == '?' ||
        c == '~' || c == '_' || c == '^' || c == '&' || isalpha(c);
}

// Helper function to determine whether the given char could be any character
// (except the first character) in a symbol
bool isSubsequentSymbol(char c) {
    return c == '+' || c == '-' || isInitialSymbol(c) || isNumber(c);
}

// Helper function to determine whether the given character marks the end of
// a token
bool isBlank(char c) {
    return c == ' ' || c == '\n' || c == EOF || c == '(' || c == ')' || c == '\"';
}

// Helper function to parse a number from the input stream
// Takes the first character of the number
// Fills in end with the first non-number character from the stream
// Fills in num with the parsed number
// Returns true if the number is an integer, false otherwise
bool parseNumber(char start, char *end, double *num) {
    assert(end);
    assert(num);
    char curChar = fgetc(stdin);
    double mult = 10.0f;
    bool decimal = false;
    double curNum;
    double total;
    if(start == '.') {
        if(!isNumber(curChar)) {
            printf("\'.\' is not a valid token\n");
            texit(10);
        }
        total = 0.0f;
        decimal = true;
        mult = 1.0f;
    }
    else total = start - '0';
    while(isNumber(curChar)) {
        if(decimal) {
            if(curChar == '.') {
                printf("A number cannot have 2 decimal points in it\n");
                texit(1);
            }
            mult *= 10.0f;
            curNum = curChar - '0';
            curNum /= mult;
            total += curNum;
        }
        else if(curChar == '.') {
            decimal = true;
            mult = 1.0f;
        }
        else {
            curNum = curChar - '0';
            total *= mult;
            total += curNum;
        }
        curChar = fgetc(stdin);
    }
    *num = total;
    *end = curChar;
    return !decimal;
}

// Helper function used to tokenize numbers
// Takes the first character of the symbol
// Takes a boolean that represents whether or not the number is negative
// Fills end with the first non-number character from the stream
// Fills val with the result from parsing
// Returns whether or not the number was valid
bool handleNumber(Value *val, char *end, char start, bool isNegative) {
    assert(val);
    assert(end);
    double num;
    bool isInt = parseNumber(start, end, &num);
    if(isNegative) num *= -1.0f;
    if(isInt) makeInteger(val, (int)num);
    else makeDouble(val, num);
    if(isBlank(*end)) return true;
    return false;
}

// Helper function to tokenize a symbol
// Takes the first character of the symbol
// Fills end with the first non-symbol character
// Fills the given Value with the results
// Returns whether or not the symbol was valid
bool handleSymbol(Value *val, char *end, char start) {
    assert(val);
    assert(end);
    SymbolString symbol;
    initSymbolString(&symbol);
    append(&symbol, start);
    char curChar = fgetc(stdin);
    while(isSubsequentSymbol(curChar)) {
        append(&symbol, curChar);
        curChar = fgetc(stdin);
    }
    *end = curChar;
    makeSymbol(val, symbol.str);
    if(isBlank(*end)) return true;
    return false;
}

// Helper function to tokenize a string
// Takes the first character of the string (aka a ")
// Fills end with the first non-string character (not the last ")
// Fills the given Value with the results
// Returns whether or not the string was valid
bool handleString(Value *val, char *end, char start) {
    assert(val);
    assert(end);
    SymbolString symbol;
    initSymbolString(&symbol);
    append(&symbol, start);
    char curChar = fgetc(stdin);
    while(curChar != '\"' && curChar != EOF && curChar != '\n') {
        append(&symbol, curChar);
        curChar = fgetc(stdin);
    }
    if(curChar == '\"') {
        *end = fgetc(stdin);
        append(&symbol, curChar);
        makeString(val, symbol.str);
        return true;
    }
    *end = curChar;
    return false;
}

// Read all of the input from stdin, and return a linked list consisting of all
// the tokens
Value *tokenize() {
    // Set up linked list
    Value *list = makeNull();
    list->type = CONS_TYPE;
    Value *tail = list;
    Value* curVal = makeNull();

    bool addToList;
    char curChar = fgetc(stdin);
    // Tokenize until the end of the file
    while(curChar != EOF) {
        addToList = true;
        // Open parenthese
        if(curChar == '(') {
            curVal->type = OPEN_TYPE;
            makeStringMalloc(curVal, "(", 1);
            curChar = fgetc(stdin);
        }
        // Close parenthese
        else if(curChar == ')') {
            curVal->type = CLOSE_TYPE;
            makeStringMalloc(curVal, ")", 1);
            curChar = fgetc(stdin);
        }
        // Comments
        else if(curChar == ';') {
            while(curChar != '\n' && curChar != EOF) curChar = fgetc(stdin);
            addToList = false;
        }
        // + / - => Symbol and Number
        else if(curChar == '-' || curChar == '+') {
            char sign = curChar;
            curChar = fgetc(stdin);
            // Number
            if(isNumber(curChar)) {
                bool isNegative = sign == '-';
                if(!handleNumber(curVal, &curChar, curChar, isNegative)) {
                    printf("%c is not a number\n", curChar);
                    texit(2);
                }
            }
            // Symbol
            else if(isBlank(curChar)) {
                curVal->type = SYMBOL_TYPE;
                char *signStr;
                if(sign == '+') signStr = "+";
                else signStr = "-";
                makeStringMalloc(curVal, signStr, 1);
            }
            // Not a valid token
            else {
                printf("Cannot start symbol with a %c\n", sign);
                texit(3);
            }
        }
        // Number
        else if(isNumber(curChar)) {
            if(!handleNumber(curVal, &curChar, curChar, false)) {
                // THROW ERROR
                printf("%c is not a number\n", curChar);
                texit(4);
            }
        }
        // Boolean
        else if(curChar == '#') {
            char boolType = fgetc(stdin);
            curChar = fgetc(stdin);
            if(!isBlank(curChar)) {
                printf("Cannot start a symbol with #\n");
                texit(5);
            }
            else if(boolType == 't') {
                makeBool(curVal, true);
            }
            else if(boolType == 'f') {
                makeBool(curVal, false);
            }
            else {
                printf("Cannot start a symbol with #\n");
                texit(6);
            }
        }
        // Symbol
        else if(isInitialSymbol(curChar)) {
            if(!handleSymbol(curVal, &curChar, curChar)) {
                printf("%c is not a valid character\n", curChar);
                texit(7);
            }
        }
        else if(curChar == '\"') {
            if(!handleString(curVal, &curChar, curChar)) {
                // THROW ERROR
                printf("Unterminated string\n");
                texit(8);
            }
        }
        // Moves on to next line
        else if(curChar == '\n') {
            curChar = fgetc(stdin);
            addToList = false;
        }
        // Moves on to next token
        else if(curChar == ' ') {
            curChar = fgetc(stdin);
            addToList = false;
        }
        // Unrecognized character
        else {
            // THROW ERROR
            printf("%c is not a valid character\n", curChar);
            texit(9);
        }

        // Puts the result into the linked list
        if(addToList) {
            setCar(tail, curVal);
            curVal = makeNull();
            curVal->type = CONS_TYPE;
            setCdr(tail, curVal);
            tail = curVal;
            curVal = makeNull();
        }
    }
    tail->type = NULL_TYPE;
    return list;
}

// Helper function to display a token
void displayTokenValue(Value *val) {
    if(val->type == INT_TYPE) printf("%i:integer\n", val->i);
    else if(val->type == STR_TYPE) printf("%s:string\n", val->s);
    else if(val->type == DOUBLE_TYPE) printf("%f:float\n", val->d);
    else if(val->type == CLOSE_TYPE) printf("%s:close\n", val->s);
    else if(val->type == OPEN_TYPE) printf("%s:open\n", val->s);
    else if(val->type == SYMBOL_TYPE) printf("%s:symbol\n", val->s);
    else if(val->type == BOOL_TYPE) {
        if(val->i) printf("#t:boolean\n");
        else printf("#f:boolean\n");
    }
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {
    Value *cur = list;
    if(cur->type == CONS_TYPE) {
        displayTokens(car(cur));
        displayTokens(cdr(cur));
    } else if (!isNull(list)) displayTokenValue(list);
}
