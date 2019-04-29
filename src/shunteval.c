// shunteval.c
// Evaluates converted expressions

#include <stdio.h>
#include <math.h>
#include "shunting.h"

int main(int argc, char** argv) {
    if(argc != 2) die("Usage: shunteval <expression>");

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

    TokenStack eval_stack;
    stack_init(&eval_stack);

    Token *t, *new;
    while(t = queue_remove(&output)) {
        if(t->type == TOKEN_NUMBER) {
            stack_push(&eval_stack, t);
        } else if(t->type == TOKEN_OPERATOR) {
            Token *a, *b;
            // remember to first pop b then a
            if(!(b = stack_pop(&eval_stack)) || !(a = stack_pop(&eval_stack))) die("Stack empty.");

            long long value;
            switch(t->v_operator) {
                case OPERATOR_PLUS:   value = a->v_number + b->v_number;      break;
                case OPERATOR_MINUS:  value = a->v_number - b->v_number;      break;
                case OPERATOR_TIMES:  value = a->v_number * b->v_number;      break;
                case OPERATOR_DIVIDE: value = a->v_number / b->v_number;      break;
                case OPERATOR_EXP:    value = powl(a->v_number, b->v_number); break;

                default: die("Unknown binary operator.");
            }

            new = malloc(sizeof(*new));
            token_init_number(new, value);
            stack_push(&eval_stack, new);
        } else if(t->type == TOKEN_UNARY) {
            Token *a = stack_pop(&eval_stack);

            long long value;
            switch(t->v_unary) {
                case UNARY_MINUS: value = -a->v_number; break;

                default: die("Unknown unary operator.");
            }

            new = malloc(sizeof(*new));
            token_init_number(new, value);
            stack_push(&eval_stack, new);
        }
    }

    Token *result = stack_pop(&eval_stack);
    if(eval_stack.top) die("Remaining operands.");

    printf("result: %d\n", result->v_number);

    return 0;
}
