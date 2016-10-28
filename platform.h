#pragma once

//TODO: this should live elsewhere
enum ConsoleColor
{
  CONSOLE_COLOR_BLACK,
  CONSOLE_COLOR_BLUE,
  CONSOLE_COLOR_GREEN,
  CONSOLE_COLOR_CYAN,
  CONSOLE_COLOR_RED,
  CONSOLE_COLOR_MAGENTA,
  CONSOLE_COLOR_BROWN,
  CONSOLE_COLOR_LIGHT_GREY,
  CONSOLE_COLOR_DARK_GREY,
  CONSOLE_COLOR_LIGHT_BLUE,
  CONSOLE_COLOR_LIGHT_GREEN,
  CONSOLE_COLOR_LIGHT_CYAN,
  CONSOLE_COLOR_LIGHT_RED,
  CONSOLE_COLOR_LIGHT_MAGENTA,
  CONSOLE_COLOR_YELLOW,
  CONSOLE_COLOR_WHITE
};

#define CONSOLE_COLOR_DEFAULT CONSOLE_COLOR_LIGHT_GREY
#define CONSOLE_COLOR_ERROR   CONSOLE_COLOR_LIGHT_RED

#if defined (_WIN32)
#include <windows.h>
#include <direct.h>
#define GETCWD _getcwd
#define CHDIR _chdir
#define FOPEN(f, fname, mode) fopen_s(&f, fname, mode)
#define stricmp _stricmp

/*
The different color codes are

0   BLACK
1   BLUE
2   GREEN
3   CYAN
4   RED
5   MAGENTA
6   BROWN
7   LIGHTGRAY
8   DARKGRAY
9   LIGHTBLUE
10  LIGHTGREEN
11  LIGHTCYAN
12  LIGHTRED
13  LIGHTMAGENTA
14  YELLOW
15  WHITE
*/

#define SET_TEXT_COLOR(c) { \
  uint32_t color_id = (uint32_t)c; \
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
  SetConsoleTextAttribute(hConsole, color_id); \
}

#else
#include <unistd.h>
#define GETCWD getcwd
#define CHDIR chdir
#define FOPEN(f, fname, mode) f = fopen(fname, mode)
#define stricmp strcasecmp
#define SET_TEXT_COLOR(C) {}
#define _strdup strdup

#endif
