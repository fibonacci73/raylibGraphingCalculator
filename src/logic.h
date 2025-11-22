#ifndef LOGIC_H
#define LOGIC_H

#include "raylib.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#define EXPRESSION_BUFFER 512
#define MAX_TOKENS 256
#define STEP 0.001

#define screenWidth 480
#define screenHeight 320
#define screenScale 0.66f

#define MAX_INTERSECTIONS 32
#define REFINE_STEPS 3 

//global variables, first defined in logic.c, they represents the minimum and maximum coordinates represented in math terms
extern float xMin, xMax, yMin, yMax, step;

int readExpression(char *expression);

void addExplicitMultiplication(char *string);
bool isOperator(char c);
bool isFunction(const char *str);
int precedence(char op);

double evaluateRPN(const char *rpn, double xValue);
void shuntingYard(char *input, char *output);

int findIntSects(char* func1, char* func2, Vector2* intersections);
int findFunctionRoots(char* func, Vector2 roots[]);
#endif