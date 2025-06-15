#include <stdio.h>
#include <ncurses.h>

#include "editor.h"

int main()
{
    Editor editor("test.txt");
    editor.start_editor();
    editor.write_content(); // Save changes before exiting
    editor.end_editor();

    return 0;
}