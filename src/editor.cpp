#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>

#include "editor.h"

Editor::Editor(const char *filename)
{
    file = fopen(filename, "r+");
    content = nullptr; // Initialize content pointer

    if (file == nullptr)
    {
        perror("Failed to open file");
    }
}

char *Editor::read_content()
{
    if (file == nullptr)
    {
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate extra space for editing (double the file size + some buffer)
    long buffer_size = (length == 0) ? 1024 : length * 2 + 1024;
    char *content = new char[buffer_size];

    if (length > 0)
    {
        fread(content, 1, length, file);
    }
    content[length] = '\0';

    return content;
}

void Editor::write_content()
{
    if (file == nullptr || content == nullptr)
    {
        return;
    }

    fseek(file, 0, SEEK_SET);
    size_t content_length = strlen(content);
    fwrite(content, sizeof(char), content_length, file);

    // Truncate file to new content length
    ftruncate(fileno(file), content_length);
    fflush(file);
}

void Editor::start_editor()
{
    // Read file content
    content = read_content();

    if (!content)
    {
        fprintf(stderr, "Failed to read file content.\n");
        return;
    }

    // Initialize ncurses
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    curs_set(1); // Make cursor visible

    int cursor_pos = 0; // Position in content buffer
    int cursor_x = 0;   // Absolute column in content
    int cursor_y = 0;   // Absolute row in content
    int scroll_x = 0;   // Horizontal scroll offset
    int scroll_y = 0;   // Vertical scroll offset

    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    std::vector<std::string> lines;
    std::string line_buffer;

    auto update_scroll = [&]()
    {
        // Minus two so the cursor does not go off the screen
        if (cursor_y > scroll_y + screen_height - 2)
        {
            scroll_y = cursor_y - (screen_height - 2);
        }
        else if (cursor_y < scroll_y)
        {
            scroll_y = cursor_y;
        }

        // Minus two so the cursor does not go off the screen
        if (cursor_x > scroll_x + screen_width - 2)
        {
            scroll_x = cursor_x - (screen_width - 2);
        }
        else if (cursor_x < scroll_x)
        {
            scroll_x = cursor_x;
        }
    };

    auto update_screen = [&]()
    {
        update_scroll();

        clear();
        lines.clear();
        line_buffer.clear();

        for (int i = 0; content[i] != '\0'; i++)
        {
            if (content[i] == '\n')
            {
                lines.push_back(line_buffer);
                line_buffer.clear();
                continue;
            }

            line_buffer += content[i];
        }

        if (!line_buffer.empty() || lines.empty())
        {
            lines.push_back(line_buffer);
        }

        // Display lines
        for (int screen_line_i = 0; screen_line_i < screen_height - 1; screen_line_i++)
        {
            int content_line_i = scroll_y + screen_line_i;

            // If end of file is reached, break
            if (content_line_i >= (int)lines.size())
            {
                break;
            }

            std::string line = lines[content_line_i];

            if (scroll_x < (int)line.length())
            {
                line = line.substr(scroll_x, screen_width);
            }
            else
            {
                line = "";
            }

            // Print the line to the screen with args (y, x, string, string) (%s to prevent line to be interpreted as a format string)
            mvprintw(screen_line_i, 0, "%s", line.c_str());
        }

        move(cursor_y - scroll_y, cursor_x - scroll_x);
        refresh();
    };

    // Initial screen update
    update_screen();

    auto cursor_left = [&]()
    {
        if (cursor_pos == 0)
        {
            return;
        }

        cursor_pos--;

        if (content[cursor_pos] == '\n')
        {
            cursor_y--;

            cursor_x = lines[cursor_y].length() > 0 ? lines[cursor_y].length() : 0;
        }
        else
        {
            cursor_x--;
        }
    };

    auto cursor_right = [&]()
    {
        if (content[cursor_pos] == '\0')
        {
            return;
        }

        cursor_pos++;

        if (content[cursor_pos - 1] == '\n')
        {
            cursor_y++;
            cursor_x = 0; // Move to the start of the next line
        }
        else
        {
            cursor_x++;
        }
    };

    auto cursor_down = [&]()
    {
        if (cursor_y + 1 >= (int)lines.size())
        {
            return; // Already at the last line
        }

        cursor_y++;
        cursor_pos += lines[cursor_y - 1].length() - cursor_x + 1; // +1 for the newline character

        if (cursor_x >= (int)lines[cursor_y].length())
        {
            cursor_x = lines[cursor_y].length();                                  // Move to the end of the line
            scroll_x = cursor_x > screen_width ? cursor_x - screen_width + 1 : 0; // Adjust horizontal scroll
        }
    };

    auto cursor_up = [&]()
    {
        if (cursor_y == 0)
        {
            return;
        }

        cursor_y--;
        cursor_pos -= (lines[cursor_y].length() + 1); // -1 for the newline character

        if (cursor_x >= (int)lines[cursor_y].length())
        {
            cursor_x = lines[cursor_y].length();                                  // Move to the end of the line
            scroll_x = cursor_x > screen_width ? cursor_x - screen_width + 1 : 0; // Adjust horizontal scroll
        }
    };

    update_screen();

    while (1)
    {
        int cur_ch = getch();

        if (cur_ch == 27) // Escape key
        {
            break;
        }
        else if (cur_ch == KEY_BACKSPACE || cur_ch == 127) // Backspace key
        {
            if (cursor_pos == 0)
            {
                continue; // Nothing to delete at the start
            }

            cursor_left();

            // Shift content to the left
            memmove(&content[cursor_pos], &content[cursor_pos + 1], strlen(&content[cursor_pos + 1]) + 1);
        }
        else if (cur_ch == KEY_LEFT)
        {
            cursor_left();
        }
        else if (cur_ch == KEY_RIGHT)
        {
            cursor_right();
        }
        else if (cur_ch == KEY_UP)
        {
            cursor_up();
        }
        else if (cur_ch == KEY_DOWN)
        {
            cursor_down();
        }

        update_screen();
    }
}

void Editor::end_editor()
{
    endwin(); // End ncurses mode

    if (content)
    {
        delete[] content;  // Free the content memory
        content = nullptr; // Set content pointer to nullptr
    }

    if (file)
    {
        fclose(file);   // Close the file
        file = nullptr; // Set file pointer to nullptr
    }
}