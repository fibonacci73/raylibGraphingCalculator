    #include "raylib.h"

    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>

    #include "gui.h"
    #include "logic.h"

    #define maxLines 8
    #define maxLineLenght 300

    int main(int argc, char const *argv[])
    {
        InitWindow(screenWidth * screenScale, screenHeight * screenScale, "window");
        SetExitKey(0); //no key will close the window

        Scene currentScene = MENU;

        char **expressions = NULL;
        int count = 0;
        
        while(!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(WHITE);

            switch(currentScene)
            {
                case MENU:
                    menu(&currentScene);
                    break;
                case EXIT:
                    CloseWindow();
                    return 0;
                case INPUT:
                    inputScene(&currentScene, &expressions, &count);
                    break;
                case GRAPHICS:
                    graphicScene(&currentScene, count, expressions);
                    break;
                case VWINDOW:
                    vWindowScene(&currentScene);
                    break;
                case GSOLVE:
                    gSolveScene(&currentScene, &count, &expressions);
            }

            EndDrawing();
        }

        count = sizeof(expressions) / sizeof(expressions[0]);
        if (expressions != NULL)
        {
            for (int i = 0; i < count; i++)
                free(expressions[i]);
            free(expressions);
        }
        CloseWindow();
    }