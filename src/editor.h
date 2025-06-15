#include <stdio.h>

class Editor
{
public:
    FILE *file;
    char *content;

    Editor(const char *filename);

    char *read_content();
    void write_content();
    void start_editor();
    void end_editor();
};