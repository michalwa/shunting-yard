// shunt.c
// Dijkstra's Shunting-Yard Algorithm
// for converting infix to postfix notation

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void die(const char *err) {
    puts(err);
    exit(1);
}

typedef enum TokenType {
    TOKEN_NUMBER      = 0,
    TOKEN_OPERATOR    = 1,
    TOKEN_PARENTHESIS = 2,
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

typedef enum Parenthesis {
    PARENTHESIS_OPEN  = 0, // (
    PARENTHESIS_CLOSE = 1, // )
} Parenthesis;

typedef struct Token Token; // declare type before definition to allow self-reference
struct Token {
    TokenType       type;
    Token          *next; // in queue/stack
    union {
        uint16_t    v_number;
        Operator    v_operator;
        Parenthesis v_parenthesis;
    };
};

void token_init_number(Token *token, uint16_t value) {
    token->type = TOKEN_NUMBER;
    token->v_number = value;
    token->next = NULL;
}

void token_init_operator(Token *token, Operator op) {
    token->type = TOKEN_OPERATOR;
    token->v_operator = op;
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
    uint16_t number;
    do {
        if(*c == 0) break;

        // numbers
        if('0' <= *c && *c <= '9') {
            number = 0;
            do {
                number = number * 10 + (*c - '0');
            } while(*(++c) && '1' <= *c && *c <= '9');
            c--; // woah, move back a little

            Token *t = malloc(sizeof(*t));
            token_init_number(t, number);
            queue_insert(input, t);

        // operators, parentheses
        } else {
            Token *t = malloc(sizeof(*t));
            parse_char(t, *c);
            queue_insert(input, t);
        }
    } while(*(++c));
}

// applies the shunting yard algorithm moving the elements from the input queue to the output queue
void shunting_yard(TokenQueue *input, TokenQueue *output) {
    TokenStack stack;
    stack_init(&stack);

    Token *t;
    while(t = queue_remove(input)) {

        // if it's a number, move it to the output queue
        if(t->type == TOKEN_NUMBER) {
            queue_insert(output, t);

        // if it's an operator...
        } else if(t->type == TOKEN_OPERATOR) {

            // pop all operators with higher or equal precedence from the stack to the output queue
            while(stack.top && stack.top->type == TOKEN_OPERATOR) {
                if(PRECEDENCE[stack.top->v_operator] < PRECEDENCE[t->v_operator]) break;
                queue_insert(output, stack_pop(&stack));
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
        }
    }

    // pop all remaining operators from the stack to the output queue
    while(t = stack_pop(&stack)) {
        if(t->type == TOKEN_PARENTHESIS && t->v_parenthesis == PARENTHESIS_OPEN) {
            die("Unmatched opening parenthesis.");
        }
        queue_insert(output, t);
    }
}

int main(int argc, char** argv) {
    if(argc != 2) die("Usage: ./shunt <expression>");

    TokenQueue input;
    queue_init(&input);
    read_input(&input, argv[1]);

    printf("input:  ");
    queue_dump(&input);

    TokenQueue output;
    queue_init(&output);
    shunting_yard(&input, &output);

    printf("output: ");
    queue_dump(&output);

    return 0;
}
