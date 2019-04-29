// shunting.h
// Shunting Yard Algorithm

#ifndef _SHUNTING_H
#define _SHUNTING_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

void die(const char *err) {
    puts(err);
    exit(1);
}

typedef enum TokenType {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_UNARY,
    TOKEN_PARENTHESIS,
} TokenType;

typedef enum Operator {
    OPERATOR_PLUS   = 0, // + addition
    OPERATOR_MINUS  = 1, // - subtraction
    OPERATOR_TIMES  = 2, // * multiplication
    OPERATOR_DIVIDE = 3, // / division
    OPERATOR_EXP    = 4, // ^ exponentiation
} Operator;

static const char    OPCHARS[]    = { '+', '-', '*', '/', '^' };
static const uint8_t PRECEDENCE[] = {  0,   0,   1,   1,   2  };
static const bool    RIGHTASSOC[] = {  0,   0,   0,   0,   1  }; // same precedence => must have same associativity

typedef enum Unary {
    UNARY_MINUS     = 0, // -n, negation
} Unary;

static const char *UNCHARS[] = { "(-)" };

typedef enum Parenthesis {
    PARENTHESIS_OPEN  = 0, // (
    PARENTHESIS_CLOSE = 1, // )
} Parenthesis;

typedef struct Token Token; // declare type before definition to allow self-reference
struct Token {
    TokenType       type;
    Token          *next; // in queue/stack
    union {
        long long   v_number;
        Operator    v_operator;
        Unary       v_unary;
        Parenthesis v_parenthesis;
    };
};

void token_init_number(Token *token, long long value) {
    token->type = TOKEN_NUMBER;
    token->v_number = value;
    token->next = NULL;
}

void token_init_operator(Token *token, Operator op) {
    token->type = TOKEN_OPERATOR;
    token->v_operator = op;
    token->next = NULL;
}

void token_init_unary(Token *token, Unary unary) {
    token->type = TOKEN_UNARY;
    token->v_unary = unary;
    token->next = NULL;
}

void token_init_parenthesis(Token *token, Parenthesis paren) {
    token->type = TOKEN_PARENTHESIS;
    token->v_parenthesis = paren;
    token->next = NULL;
}

// prints a textual representation of the token and a space
void print_token(Token *token) {
    if(token->type == TOKEN_NUMBER) {
        printf("%d ", token->v_number);
    } else if(token->type == TOKEN_OPERATOR) {
        printf("%c ", OPCHARS[token->v_operator]);
    } else if(token->type == TOKEN_UNARY) {
        printf("%s ", UNCHARS[token->v_unary]);
    } else if(token->type == TOKEN_PARENTHESIS) {
        printf("%c ", token->v_parenthesis == PARENTHESIS_OPEN ? '(' : ')');
    }
}

typedef struct TokenQueue {
    Token *head; // first token or null
    Token *tail; // for efficient operations
} TokenQueue;

void queue_init(TokenQueue *queue) {
    queue->head = NULL;
    queue->tail = NULL;
}

// inserts the token into the queue
void queue_insert(TokenQueue *queue, Token *token) {
    token->next = NULL;
    if(queue->head) {
        queue->tail->next = token;
    } else {
        queue->head = token;
    }
    queue->tail = token;
}

// removes and returns a token from the head of the queue
Token *queue_remove(TokenQueue *queue) {
    if(!queue->head) return NULL;
    Token *head = queue->head;
    queue->head = head->next;
    if(!queue->head) queue->tail = NULL;
    head->next = NULL;
    return head;
}

// prints out all elements of the queue
void queue_dump(TokenQueue *queue) {
    if(!queue->head) return;
    Token *t = queue->head;
    do print_token(t); while(t = t->next);
    puts("");
}

typedef struct TokenStack {
    Token *top; // top token or null
} TokenStack;

void stack_init(TokenStack *stack) {
    stack->top = NULL;
}

// pushes a token onto the stack
void stack_push(TokenStack *stack, Token *token) {
    if(stack->top) {
        token->next = stack->top;
    } else {
        token->next = NULL;
    }
    stack->top = token;
}

// pops and returns a token off the stack
Token *stack_pop(TokenStack *stack) {
    if(!stack->top) return NULL;
    Token *t = stack->top;
    stack->top = stack->top->next;
    return t;
}

// parses a single character into a token
void parse_char(Token *t, char c) {
    switch(c) {
        case '+': token_init_operator(t, OPERATOR_PLUS);        break;
        case '-': token_init_operator(t, OPERATOR_MINUS);       break;
        case '*': token_init_operator(t, OPERATOR_TIMES);       break;
        case '/': token_init_operator(t, OPERATOR_DIVIDE);      break;
        case '^': token_init_operator(t, OPERATOR_EXP);         break;

        case '(': token_init_parenthesis(t, PARENTHESIS_OPEN);  break;
        case ')': token_init_parenthesis(t, PARENTHESIS_CLOSE); break;

        default:
            die("Unexpected character.");
    }
}

void read_input(TokenQueue *input, char *c) {
    bool lastreadop = true;
    long long number;
    do {
        if(*c == 0) break;

        // minus sign
        if(lastreadop && *c == '-') {
            Token *t = malloc(sizeof(*t));
            token_init_unary(t, UNARY_MINUS);
            queue_insert(input, t);
        }

        // numbers
        else if('0' <= *c && *c <= '9') {
            number = 0;
            do {
                number = number * 10 + (*c - '0');
            } while(*(++c) && '1' <= *c && *c <= '9');
            c--; // woah, move back a little

            Token *t = malloc(sizeof(*t));
            token_init_number(t, number);
            queue_insert(input, t);

            lastreadop = false;
        }

        // operators, parentheses
        else {
            Token *t = malloc(sizeof(*t));
            parse_char(t, *c);
            queue_insert(input, t);

            if(t->type == TOKEN_OPERATOR || t->type == TOKEN_PARENTHESIS && t->v_parenthesis == PARENTHESIS_OPEN) {
                lastreadop = true;
            } else {
                lastreadop = false;
            }
        }
    } while(*(++c));
}

// applies the shunting yard algorithm moving the elements from the input queue to the output queue
void shunting_yard(TokenQueue *input, TokenQueue *output) {
    TokenStack stack;
    stack_init(&stack);

    Token *last_read = NULL;
    Token *t;
    while(t = queue_remove(input)) {

        // if it's a number, move it to the output queue
        if(t->type == TOKEN_NUMBER) {
            queue_insert(output, t);

        // if it's an operator...
        } else if(t->type == TOKEN_OPERATOR) {

            // pop all operators with higher or equal precedence and left associativity from the stack to the output queue
            while(stack.top && stack.top->type == TOKEN_OPERATOR) {
                if(PRECEDENCE[stack.top->v_operator] > PRECEDENCE[t->v_operator] ||
                (PRECEDENCE[stack.top->v_operator] == PRECEDENCE[t->v_operator] && !RIGHTASSOC[stack.top->v_operator])) {
                    queue_insert(output, stack_pop(&stack));
                }
                else break;
            }

            // and then push the operator onto the stack
            stack_push(&stack, t);
        } else if(t->type == TOKEN_PARENTHESIS) {

            // if it's an opening parenthesis, push it onto the stack
            if(t->v_parenthesis == PARENTHESIS_OPEN) {
                stack_push(&stack, t);

            // if it's a closing parenthesis...
            } else if(t->v_parenthesis == PARENTHESIS_CLOSE) {
                
                // pop all operators from the stack to the output queue until an opening parenthesis is found
                while(stack.top) {
                    if(stack.top->type == TOKEN_PARENTHESIS && stack.top->v_parenthesis == PARENTHESIS_OPEN) break;
                    queue_insert(output, stack_pop(&stack));
                }

                // pop the opening parenthesis as well, discard the closing parenthesis
                if(stack.top && stack.top->type == TOKEN_PARENTHESIS && stack.top->v_parenthesis == PARENTHESIS_OPEN) {
                    stack_pop(&stack);
                } else {
                    die("Unmatched closing parenthesis.");
                }
            }

        // unary operator -> push onto the stack
        } else if(t->type == TOKEN_UNARY) {
            stack_push(&stack, t);

        } else {
            die("Unsupported token type.");
        }

        last_read = t;
    }

    // pop all remaining operators from the stack to the output queue
    while(t = stack_pop(&stack)) {
        if(t->type == TOKEN_PARENTHESIS && t->v_parenthesis == PARENTHESIS_OPEN) {
            die("Unmatched opening parenthesis.");
        }
        queue_insert(output, t);
    }
}

#endif // _SHUNTING_H
