#include <stdio.h>
#include <ncurses.h>

#include "editor.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Initialize the editor with the provided filename
    const char *filename = argv[1];

    // Create an instance of the Editor class
    Editor editor(filename);

    // Start the editor
    editor.start_editor();
    // Write content to the file before exiting
    editor.write_content(); // Save changes before exiting
    // End the editor session
    editor.end_editor();

    return 0;
}