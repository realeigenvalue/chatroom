#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define MSG_SIZE 256

int closeChat;

static WINDOW *output;
static WINDOW *input;
static WINDOW *boundary;

void close_chat() { closeChat = 1; }

void draw_border(WINDOW *screen) {
  int x, y, i;
  getmaxyx(screen, y, x);
  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, y - 1, i, "-");
  }
}

void create_windows() {

  int parent_x, parent_y;
  int boundary_size = 2;
  initscr();
  curs_set(1); // Sets cursor visibility

  // Get maximum window dimensions
  getmaxyx(stdscr, parent_y, parent_x);

  // Set up initial windows
  output = newwin(parent_y - boundary_size - 1, parent_x, 0, 0);
  input = newwin(1, parent_x, parent_y - 1, 0);
  boundary = newwin(boundary_size, parent_x, parent_y - boundary_size - 1, 0);
  // Allow the windows to scroll
  scrollok(output, TRUE);
  scrollok(input, TRUE);

  draw_border(boundary);
  wrefresh(boundary);
}

void write_message_to_screen(const char *format, ...) {

  va_list arg;
  va_start(arg, format);
  vw_printw(output, format, arg);
  va_end(arg);
  wrefresh(output);
}

void read_message_from_screen(char **buffer) {
  if (*buffer == NULL)
    *buffer = calloc(1, MSG_SIZE);
  else
    memset(*buffer, 0, MSG_SIZE);
  wprintw(input, "> ");
  wrefresh(input);
  wgetnstr(input, *buffer, MSG_SIZE - 1);
  if (closeChat) {
    free(*buffer);
    *buffer = NULL;
  }
}

void destroy_windows() {
  // clean up
  delwin(output);
  delwin(boundary);
  delwin(input);
  endwin();
}