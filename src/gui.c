#include "gui.h"
#include "logic.h"

#include <string.h>
#include <stdio.h>

const char *scenesName[scenesNum] = {"1. Menu", "2. Graphic", "3. Settings"};
const char *gSolveOptionsName[scenesNum] = {"1. Intesect", "2. Root", "3. Ysept"};

//implements the general menuSample() function for the main menu
void menu(Scene *nextScene)
{
    static int selectedOption = 0;  // persistent across frames

    menuSample(scenesName, scenesNum, &selectedOption);

    if (IsKeyPressed(KEY_ENTER))
    {
        *nextScene = (Scene)selectedOption;
    }
}

// draws the buttons in menu, they can be hovered and changes colors when they are so
void drawMenuButton(const char *text, Vector2 position, bool isClicked)
{
    Color textBackGroundColor = isClicked ? SKYBLUE : BLANK;
    int textWidth = MeasureText(text, menuFontSize);
    DrawRectangle(position.x, position.y, textWidth + 2, menuFontSize, textBackGroundColor);
    DrawText(text, position.x, position.y, menuFontSize, BLACK);
}

// the whole scene in which graphics are drew, it's a variadic function. allowing virtually infinite functions as parameters
void graphicScene(Scene *nextScene, int functionsNumber, char **expressions)
{
    Color palette[] = {RED, GREEN, BLUE, ORANGE, PURPLE};
    const int paletteSize = sizeof(palette) / sizeof(palette[0]);
    char parsed[EXPRESSION_BUFFER];

    drawAxes();

    for (int i = 0; i < functionsNumber; i++)
    {
        shuntingYard(expressions[i], parsed);
        if (parsed[0] != '\0')
        {
            drawFunction(parsed, palette[i % paletteSize]);
        }
    }

    if (IsKeyPressed(KEY_ESCAPE))
        *nextScene = INPUT;
    if (IsKeyPressed(KEY_V))
        *nextScene = VWINDOW;
    if (IsKeyPressed(KEY_G))
        *nextScene = GSOLVE;
}

// it's called in graphicScene(), its purpose is to draw the graph of a function, sliding between each x value, calculating the corresponding y value
void drawFunction(const char *rpn, Color color)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    int steps = (int)((xMax - xMin) / STEP);

    double scaleX = width / (xMax - xMin);
    double scaleY = height / (yMax - yMin);

    float x0 = -xMin / (xMax - xMin) * width;
    float y0 = height - (-yMin / (yMax - yMin) * height);

    for (int i = 0; i <= steps; i++)
    {
        double x = xMin + i * STEP;
        double y = evaluateRPN(rpn, x);

        int screenX = (int)(x0 + x * scaleX);
        int screenY = (int)(y0 - y * scaleY);

        if (screenX >= 0 && screenX < width &&
            screenY >= 0 && screenY < height)
        {
            DrawPixel(screenX, screenY, color);
        }
    }
}

// draws axes in graphicScene(), it also draws the grid
void drawAxes()
{
    // Calculate window width and height in pixels (considering scale)
    float width = screenWidth * screenScale;
    float height = screenHeight * screenScale;

    // Calculate pixel coordinates of the mathematical origin (0,0)
    float x0 = -xMin / (xMax - xMin) * width;
    float y0 = height - (-yMin / (yMax - yMin) * height);

    // Compute scale factor (pixels per unit)
    float xScale = width / (xMax - xMin);
    float yScale = height / (yMax - yMin);

    // Compute “nice” step sizes for X and Y axes
    float stepX = 1;
    float stepY = 1;

    // --- Draw background grid (light gray, unobtrusive) ---
    // Vertical lines
    for (float x = 0; x <= xMax; x += stepX)
    {
        float px = (x - xMin) * xScale;
        DrawLine(px, 0, px, height, LIGHTGRAY);
    }
    for (float x = -stepX; x >= xMin; x -= stepX)
    {
        float px = (x - xMin) * xScale;
        DrawLine(px, 0, px, height, LIGHTGRAY);
    }

    // Horizontal lines
    for (float y = 0; y <= yMax; y += stepY)
    {
        float py = height - (y - yMin) * yScale;
        DrawLine(0, py, width, py, LIGHTGRAY);
    }
    for (float y = -stepY; y >= yMin; y -= stepY)
    {
        float py = height - (y - yMin) * yScale;
        DrawLine(0, py, width, py, LIGHTGRAY);
    }

    // --- Draw X and Y axes (darker gray for contrast) ---
    DrawLine(0, (int)y0, (int)width, (int)y0, GRAY);  // X-axis
    DrawLine((int)x0, 0, (int)x0, (int)height, GRAY); // Y-axis

    // --- Draw small arrowheads at the end of each axis ---
    DrawTriangle((Vector2){width - 10, y0 - 4}, (Vector2){width - 10, y0 + 4}, (Vector2){width, y0}, GRAY); // X arrow
    DrawTriangle((Vector2){x0 - 4, 10}, (Vector2){x0 + 4, 10}, (Vector2){x0, 0}, GRAY);                     // Y arrow

    // --- Axis labels ---
    DrawText("X", width - 15, y0 + 5, 10, GRAY);
    DrawText("Y", x0 + 5, 5, 10, GRAY);
}

void inputScene(Scene *nextScene, char ***expressions, int *count)
{
    static int selected = 0;                    // indice riga selezionata
    static bool editing = false;                // se stai modificando una riga
    static char buffer[EXPRESSION_BUFFER] = ""; // buffer temporaneo

    // Tavolozza condivisa con graphicScene()
    Color palette[] = {RED, GREEN, BLUE, ORANGE, PURPLE};
    const int paletteSize = sizeof(palette) / sizeof(palette[0]);

    // Alloca le espressioni iniziali se serve
    if (*expressions == NULL)
    {
        *expressions = calloc(MAX_FUNCTIONS, sizeof(char *));
        *count = MAX_FUNCTIONS;
        for (int i = 0; i < MAX_FUNCTIONS; i++)
        {
            (*expressions)[i] = calloc(EXPRESSION_BUFFER, sizeof(char));
            (*expressions)[i][0] = '\0';
        }
    }

    // ──────────────────────────────
    // editing
    // ──────────────────────────────
    if (editing)
    {
        readExpression(buffer);
        DrawText(buffer, 40, 10, 20, palette[selected % paletteSize]);

        if (IsKeyPressed(KEY_ENTER))
        {
            strcpy((*expressions)[selected], buffer);
            buffer[0] = '\0';
            editing = false;
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            buffer[0] = '\0';
            editing = false;
        }

        return;
    }

    // ──────────────────────────────
    // navigation
    // ──────────────────────────────

    for (int i = 0; i < *count; i++)
    {
        Color color = palette[i % paletteSize];
        bool isSelected = (i == selected);

        // Evidenzia la riga selezionata
        if (isSelected)
        {
            DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(LIGHTGRAY, 0.4f));
        }

        char label[16];
        snprintf(label, sizeof(label), "Y%d =", i + 1);

        // Selezionata → testo più grande o più acceso
        DrawText(label, 20, 10 + 30 * i, 20, color);
        DrawText((*expressions)[i], 80, 10 + 30 * i, 20, color);
    }

    // Frecce per navigare
    if (IsKeyPressed(KEY_DOWN))
        selected = (selected + 1) % *count;

    if (IsKeyPressed(KEY_UP))
        selected = (selected - 1 + *count) % *count;

    // Enter → modifica
    if (IsKeyPressed(KEY_ENTER))
    {
        strcpy(buffer, (*expressions)[selected]);
        editing = true;
    }

    if (IsKeyPressed(KEY_DELETE))
    {
        (*expressions)[selected][0] = '\0';
    }

    // Spazio → vai alla scena grafica
    if (IsKeyPressed(KEY_SPACE))
    {
        int nonEmptyCount = 0;
        for (int i = 0; i < *count; i++)
            if (strlen((*expressions)[i]) > 0)
                nonEmptyCount++;

        if (nonEmptyCount > 0)
            *nextScene = GRAPHICS;
    }

    // Esc → torna al menu e pulisci
    if (IsKeyPressed(KEY_ESCAPE))
    {
        for (int i = 0; i < *count; i++)
            free((*expressions)[i]);
        free(*expressions);
        *expressions = NULL;
        *count = 0;

        *nextScene = MENU;
    }
}

void vWindowScene(Scene *nextScene)
{
    static int selected = 0;
    static bool editing = false;
    static char buffer[32] = "";

    const char *labels[4] = {"Xmin =", "Xmax =", "Ymin =", "Ymax ="};
    float *vars[4] = {&xMin, &xMax, &yMin, &yMax};

    // INPUT
    if (!editing)
    {
        if (IsKeyPressed(KEY_DOWN))
            selected = (selected + 1) % 4;
        if (IsKeyPressed(KEY_UP))
            selected = (selected - 1 + 4) % 4;

        if (IsKeyPressed(KEY_ENTER))
        {
            editing = true;
            snprintf(buffer, sizeof(buffer), "%.2f", *vars[selected]);
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            *nextScene = GRAPHICS;
        }
    }
    else
    {
        // Lettura caratteri premuti
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && key <= 126 && strlen(buffer) < sizeof(buffer) - 1)
            {
                size_t len = strlen(buffer);
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
            }
            key = GetCharPressed(); // legge tutti i caratteri premuti nello stesso frame
        }

        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE) && strlen(buffer) > 0)
        {
            buffer[strlen(buffer) - 1] = '\0';
        }

        // Conferma
        if (IsKeyPressed(KEY_ENTER))
        {
            *vars[selected] = atof(buffer);
            buffer[0] = '\0';
            editing = false;
        }
    }

    // DRAW
    DrawText("Modifica finestra grafico", 10, 10, 24, BLACK);

    for (int i = 0; i < 4; i++)
    {
        bool isSelected = (i == selected);
        Color color = isSelected ? LIGHTGRAY : DARKGRAY;

        DrawText(labels[i], 10, 40 + 30 * i, 20, color);

        char valText[32];
        if (isSelected && editing)
            DrawText(buffer, 80, 40 + 30 * i, 20, BLACK);
        else
        {
            snprintf(valText, sizeof(valText), "%.2f", *vars[i]);
            DrawText(valText, 80, 40 + 30 * i, 20, BLACK);
        }
    }
}

void gSolveScene(Scene *nextScene, int *count, char ***expressions)
{
    static int selectedOption = 0;  // persistent across frames
    static bool isOptionSelected = false;

    if(!isOptionSelected) //if options is still not selected
        menuSample(gSolveOptionsName, scenesNum, &selectedOption);

    if(IsKeyPressed(KEY_ENTER))
        isOptionSelected = true;
    
    if(IsKeyPressed(KEY_ESCAPE) && !isOptionSelected) //differentiated from the esc in switch: case 0
    {
        *nextScene = GRAPHICS;
        return;
    }


    if(isOptionSelected)
    {
        static bool justEntered = true;
        switch (selectedOption)
        {
        case 0: //IntSect code
            
            Color palette[] = {RED, GREEN, BLUE, ORANGE, PURPLE};
            const int paletteSize = sizeof(palette) / sizeof(palette[0]);

            static int hovered = 0; 
            static int funcs[2] = {-1, -1};
            static int stage = 0;

            static Vector2 intersections[MAX_INTERSECTIONS];
            static int intsectsNum = 0;

<<<<<<< HEAD
            static bool calcJustEntered = true;
=======
            static bool firstCalcIntSects = true;
>>>>>>> 7d1d86ab53232ed60290034406a8741afcc72f8b


            //Ignores first frame input
            if(justEntered) 
            {
                //resets the static variables if is the first frame
                hovered = 0;
                funcs[0] = -1;
                funcs[1] = -1;
                stage = 0;

                justEntered = false;
                break; //exits right away
            }

            if(IsKeyPressed(KEY_DOWN))
            {
                hovered++;
            }

            if(IsKeyPressed(KEY_UP))
            {
                hovered--;
            }

            if(IsKeyPressed(KEY_ENTER) && stage < 2)
            {
                //doesn't allow to pick a empty function, doesn't allow to pick the same expression twice
                if(funcs[0] == hovered || (*expressions)[hovered][0] == '\0') break;
                funcs[stage] = hovered;
                stage++;
            }

            //comes back to gSolve menu
            if(IsKeyPressed(KEY_ESCAPE))
            {
                isOptionSelected = false;
                justEntered = true;
                firstCalcIntSects = true;
                break;
            }

            if(stage == 2)
<<<<<<< HEAD
            {   
                if (calcJustEntered)
                {
                    intsectsNum = findIntSects((*expressions)[funcs[0]], (*expressions)[funcs[1]], intersections);
                    calcJustEntered = false;
                }
=======
            {
                if(firstCalcIntSects)
                {
                    intsectsNum = findIntersections(
                    (*expressions)[funcs[0]], 
                    (*expressions)[funcs[1]], 
                    intersections
                    );

                    firstCalcIntSects = false;
                }

>>>>>>> 7d1d86ab53232ed60290034406a8741afcc72f8b
                
                for(int i = 0; i < intsectsNum; i++)
                {
                    char label[64];
                    snprintf(label, sizeof(label), "int %d = (%.2f, %.2f)", 
                            i + 1, intersections[i].x, intersections[i].y);
                    
                    DrawText(label, 20, 10 + 30 * i, 20, BLACK);
                }
                
                break;
            }

            for (int i = 0; i < *count; i++)
            {
                Color color = palette[i % paletteSize];

                //Highlights the hovered
                if (hovered == i)
                {
                    DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(LIGHTGRAY, 0.4f));
                }

                if(funcs[0] == i || funcs[1] == i)
                {
                    DrawRectangle(10, 5 + 30 * i, 700, 28, Fade(SKYBLUE, 0.4f));
                }

                char label[16];
                snprintf(label, sizeof(label), "Y%d =", i + 1);

                //highlights
                DrawText(label, 20, 10 + 30 * i, 20, color);
                DrawText((*expressions)[i], 80, 10 + 30 * i, 20, color);
            }


            break;
        default:
            break;
        }
    }
}

void menuSample(const char** optionsName, int optionsNum, int *selected)
{
    Vector2 startingPos = {20, 20};

    // Navigation
    if (IsKeyPressed(KEY_DOWN))
        *selected = (*selected + 1 >= optionsNum) ? 0 : *selected + 1;

    if (IsKeyPressed(KEY_UP))
        *selected = (*selected == 0) ? optionsNum - 1 : *selected - 1;

    // Draw menu
    for (int i = 0; i < optionsNum; i++)
    {
        Vector2 btnPos = {startingPos.x, startingPos.y + i * menuFontSize};
        drawMenuButton(optionsName[i], btnPos, *selected == i);
    }
}