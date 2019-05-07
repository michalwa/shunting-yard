// shunt.c
// Dijkstra's Shunting-Yard Algorithm
// for converting infix to postfix notation

#include <stdio.h>
#include "shunting.h"

int main(int argc, char** argv) {
    if(argc != 2) die("Usage: %s <expression>\n", argv[0]);

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
