#include "logic.h"

float xMin = -6.0f, xMax = 6.0f, yMin = -6.0f, yMax = 6.0f, step = 0.001f;

int readExpression(char *expression)
{
    int nextChar = GetCharPressed();
    if (nextChar == 0)
        return 0; // nessun input

    size_t len = strlen(expression);
    if (len >= EXPRESSION_BUFFER - 1)
        return 0; // buffer pieno

    // caratteri ammessi
    if ((nextChar >= '0' && nextChar <= '9') ||
        (nextChar >= 'a' && nextChar <= 'z') ||
        nextChar == '+' || nextChar == '-' ||
        nextChar == '*' || nextChar == '/' ||
        nextChar == '^' || nextChar == '|' || 
        nextChar == '(' || nextChar == ')')
    {
        expression[len] = (char)nextChar;
        expression[len + 1] = '\0';
        return 1;
    }

    // backspace
    if (nextChar == 8) // KEY_BACKSPACE
    {
        if (len > 0)
            expression[len - 1] = '\0';
        return 1;
    }

    return 0;
}

int precedence(char op)
{
    if (op == '+' || op == '-')
        return 1;
    if (op == '*' || op == '/')
        return 2;
    if (op == '^')
        return 3;
    return 0;
}


//Checks if a character is a valid operator
int isOperator(char c)
{
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

//Checks if string is a function name
int isFunction(const char *str)
{
    return (strcmp(str, "sin") == 0 || strcmp(str, "cos") == 0 ||
            strcmp(str, "tan") == 0 || strcmp(str, "abs") == 0);
}

/*
    Shunting Yard algorithm - converts infix notation to RPN
    Supports: ^, sin, cos, tan, abs (valore assoluto ||)
    Input: Mathematical expression in infix notation (e.g., "sin(x)+3^2")
    Output: Expression in Reverse Polish Notation (e.g., "x sin 3 2 ^ +")
*/
void shuntingYard(const char *input, char *output)
{
    char operatorStack[EXPRESSION_BUFFER];
    int top = -1;
    int outPos = 0;

    output[0] = '\0';

    char preprocessed[EXPRESSION_BUFFER];
    int prePos = 0;
    int absOpen = 0; // 0 = chiuso, 1 = aperto

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '|')
        {
            if (absOpen == 0) // Opening |
            {
                strcpy(preprocessed + prePos, "abs(");
                prePos += 4;
                absOpen = 1;
            }
            else // Closing |
            {
                preprocessed[prePos++] = ')';
                absOpen = 0;
            }
        }
        else
        {
            preprocessed[prePos++] = input[i];
        }
    }
    preprocessed[prePos] = '\0';

    // Usa la stringa preprocessata
    input = preprocessed;
    int len = strlen(input);
    for (int i = 0; i < len; i++)
    {
        char token = input[i];

        // Skip whitespace characters
        if (isspace((unsigned char)token))
            continue;

        // Handle numbers (including negative numbers and decimals)
        if (isdigit((unsigned char)token) ||
            (token == '-' && (i == 0 || input[i - 1] == '(' || isOperator(input[i - 1])) &&
             i + 1 < len && isdigit((unsigned char)input[i + 1])))
        {
            int start = i;
            if (token == '-')
                i++; // Include the minus sign
            // Continue reading digits and decimal points
            while (i < len && (isdigit((unsigned char)input[i]) || input[i] == '.'))
                i++;
            int numLen = i - start;
            // Copy the number to output
            strncpy(output + outPos, input + start, numLen);
            outPos += numLen;
            output[outPos++] = ' ';
            output[outPos] = '\0';
            i--; // Adjust for loop increment
        }

        // Handle functions (sin, cos, tan) and variable x
        else if (isalpha((unsigned char)token))
        {
            char func[10];
            int funcPos = 0;

            // Read the whole function name or variable
            while (i < len && isalpha((unsigned char)input[i]))
            {
                func[funcPos++] = input[i++];
            }
            func[funcPos] = '\0';
            i--; // Adjust for loop increment

            // Check if it's a function
            if (isFunction(func))
            {
                // Push a marker (like 'A' for abs, 'S' for sin, etc.)
                if (strcmp(func, "abs") == 0)
                    operatorStack[++top] = 'A';
                else if (strcmp(func, "sin") == 0)
                    operatorStack[++top] = 'S';
                else if (strcmp(func, "cos") == 0)
                    operatorStack[++top] = 'C';
                else if (strcmp(func, "tan") == 0)
                    operatorStack[++top] = 'T';
            }
            else // It's a variable (x)
            {
                strcpy(output + outPos, func);
                outPos += strlen(func);
                output[outPos++] = ' ';
                output[outPos] = '\0';
            }
        }

        // Handle opening parenthesis
        else if (token == '(')
        {
            operatorStack[++top] = token;
        }

        // Handle closing parenthesis
        else if (token == ')')
        {
            // Pop operators until we find the matching opening parenthesis
            while (top >= 0 && operatorStack[top] != '(')
            {
                output[outPos++] = operatorStack[top--];
                output[outPos++] = ' ';  // <-- ADD SPACE after each operator
                output[outPos] = '\0';
            }
            // Remove the opening parenthesis from stack
            if (top >= 0 && operatorStack[top] == '(')
                top--;
            
            // Check if there's a function marker (A, S, C, T)
            if (top >= 0 && (operatorStack[top] == 'A' || operatorStack[top] == 'S' || 
                            operatorStack[top] == 'C' || operatorStack[top] == 'T'))
            {
                output[outPos++] = operatorStack[top--];
                output[outPos++] = ' ';  // <-- IMPORTANT: Add space after function marker
                output[outPos] = '\0';
            }
        }

        // Handle operators (+, -, *, /, ^)
        else if (isOperator(token))
        {
            // ^ is right-associative, others are left-associative
            if (token == '^')
            {
                while (top >= 0 && isOperator(operatorStack[top]) &&
                       precedence(operatorStack[top]) > precedence(token))
                {
                    output[outPos++] = operatorStack[top--];
                    output[outPos++] = ' ';
                    output[outPos] = '\0';
                }
            }
            else
            {
                while (top >= 0 && isOperator(operatorStack[top]) &&
                       precedence(operatorStack[top]) >= precedence(token))
                {
                    output[outPos++] = operatorStack[top--];
                    output[outPos++] = ' ';
                    output[outPos] = '\0';
                }
            }
            // Push current operator onto stack
            operatorStack[++top] = token;
        }
    }

    // Pop remaining operators from stack
    while (top >= 0)
    {
        if (operatorStack[top] != '(' && operatorStack[top] != ')')
        {
            output[outPos++] = operatorStack[top--];
            output[outPos++] = ' ';
            output[outPos] = '\0';
        }
        else
        {
            top--;
        }
    }
}

/*
    Evaluates a Reverse Polish Notation expression
    Input: RPN expression string and value for variable x
    Output: Calculated result as double
*/
double evaluateRPN(const char *rpn, double xValue)
{
    double stack[MAX_TOKENS];
    int top = -1; // Stack pointer
    char token[50];
    int pos = 0;

    // Parse the RPN expression token by token
    for (int i = 0;; i++)
    {
        char c = rpn[i];

        // Token delimiter (space or end of string)
        if (isspace((unsigned char)c) || c == '\0')
        {
            if (pos > 0)
            {
                token[pos] = '\0';
                pos = 0;

                // Handle numbers (including negative numbers)
                if (isdigit((unsigned char)token[0]) ||
                    (token[0] == '-' && isdigit((unsigned char)token[1])))
                {
                    stack[++top] = atof(token);
                }

                // Handle variable 'x'
                else if (strcmp(token, "x") == 0)
                {
                    stack[++top] = xValue;
                }

                // Handle sin function
                else if (token[0] == 'A' && token[1] == '\0')
                {
                    if (top >= 0)
                    {
                        double a = stack[top--];
                        stack[++top] = fabs(a);
                    }
                }

                // Similarly for the other function markers:
                else if (token[0] == 'S' && token[1] == '\0')
                {
                    if (top >= 0)
                    {
                        double a = stack[top--];
                        stack[++top] = sin(a);
                    }
                }
                
                else if (token[0] == 'r' && token[1] == '\0')
                {
                    if (top >= 0)
                    {
                        double a = stack[top--];
                        stack[++top] = sqrt(a);
                    }
                }

                else if (token[0] == 'C' && token[1] == '\0')
                {
                    if (top >= 0)
                    {
                        double a = stack[top--];
                        stack[++top] = cos(a);
                    }
                }

                else if (token[0] == 'T' && token[1] == '\0')
                {
                    if (top >= 0)
                    {
                        double a = stack[top--];
                        stack[++top] = tan(a);
                    }
                }

                // Handle addition operator
                else if (strcmp(token, "+") == 0)
                {
                    if (top >= 1)
                    {
                        double b = stack[top--];
                        double a = stack[top--];
                        stack[++top] = a + b;
                    }
                }

                // Handle subtraction operator
                else if (strcmp(token, "-") == 0)
                {
                    if (top >= 1)
                    {
                        double b = stack[top--];
                        double a = stack[top--];
                        stack[++top] = a - b;
                    }
                }

                // Handle multiplication operator
                else if (strcmp(token, "*") == 0)
                {
                    if (top >= 1)
                    {
                        double b = stack[top--];
                        double a = stack[top--];
                        stack[++top] = a * b;
                    }
                }

                // Handle division operator
                else if (strcmp(token, "/") == 0)
                {
                    if (top >= 1)
                    {
                        double b = stack[top--];
                        double a = stack[top--];
                        if (b != 0)
                            stack[++top] = a / b;
                        else
                            stack[++top] = 0; // Division by zero protection
                    }
                }

                // Handle power operator ^
                else if (strcmp(token, "^") == 0)
                {
                    if (top >= 1)
                    {
                        double b = stack[top--];
                        double a = stack[top--];
                        stack[++top] = pow(a, b);
                    }
                }
            }
            // Break if we reached the end of the string
            if (c == '\0')
                break;
        }
        else
        {
            // Build the current token character by character
            token[pos++] = c;
        }
    }

    // Return the final result (top of the stack)
    return (top >= 0) ? stack[top] : 0;
}

int findIntersections(const char* expr1, const char* expr2, Vector2* intersections)
{
    float prevX = xMin;
    float y1_prev = evaluateRPN(expr1, prevX);
    float y2_prev = evaluateRPN(expr2, prevX);
    float delta_prev = y1_prev - y2_prev;

    int intSectIndex = 0;

    for (float x = xMin + step; x <= xMax; x += step)
    {
        float y1 = evaluateRPN(expr1, x);
        float y2 = evaluateRPN(expr2, x);
        float delta = y1 - y2;

        // rilevazione cambio di segno
        if ((delta_prev > 0 && delta < 0) ||
            (delta_prev < 0 && delta > 0))
        {
            if (intSectIndex < MAX_INTERSECTIONS)
            {
                // INTERPOLAZIONE LINEARE
                float t = fabsf(delta_prev) / (fabsf(delta_prev) + fabsf(delta));
                float x_int = prevX + t * (x - prevX);
                float y_int = evaluateRPN(expr1, x_int);

                intersections[intSectIndex++] = (Vector2){x_int, y_int};
            }
        }

        // sposta in avanti
        prevX = x;
        delta_prev = delta;
    }

    return intSectIndex;
}