// read,STDIN_FILENO
#include <unistd.h>

//
#include <stdio.h>

// termios,tcgetattr,tcsetattr,
// TCSAFLUSH,ECHO,ICANON
#include <termios.h>

// atexit
#include <stdlib.h>
// iscntrl
#include <ctype.h>

#include <sys/ioctl.h>
#include <string.h>

#define ctrl_value(key) ((key)&0x1f)

typedef struct erow
{
  int size;
  char *chars;
} erow;

struct terminal_config
{
  struct termios default_terminos;
  int rows;
  int cols;
  int x_position, y_position;
  int numrows;
  erow *row;
};

struct terminal_config config;

struct initial_string
{
  char *val;
  int length;
};

enum arrow_keys
{
  left,
  right,
  up,
  down
};

void enable_default();
void initial_setting();
void append_string(struct initial_string *source, char *val, int len);
void initial_rendering(struct initial_string *v, int argc, char *argv[]);
char read_key();
void key_process(char key_val);
void move_cursor(char key_val);

void enable_default()
{
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.default_terminos);
}
void initial_setting()
{
  tcgetattr(STDIN_FILENO, &config.default_terminos);
  struct termios raw = config.default_terminos;
  /* ECHO is responsible for printing the input
   ICANON is reponsible for reading line by line that is it waits for the enter key
  ISIG ctrl c and z which are used for terminating and suspending
  IXON ctrl s stops data from being transmitted untill ctrl q is pressed belongs to input flag
  IEXTEN ctrl v waits for you to enter another character
  ICRNL is for ctrl m
  OPOST Post-process output */
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_iflag &= ~(IXON | ICRNL);
  raw.c_oflag &= ~(OPOST);
  // raw.c_cc[VMIN] = 0;
  // raw.c_cc[VTIME] = 10;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  // Store the windows size
  struct winsize ws;
  printf("%d", ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws));
  config.rows = ws.ws_row;
  config.cols = ws.ws_col;
  config.x_position = 1;
  config.y_position = 1;
  config.numrows = 0;

  atexit(enable_default);
}

char read_key()
{
  char c;
  read(STDIN_FILENO, &c, 1);
  if (c == '\x1b')
  {
    char key_pressed[3];
    if (read(STDIN_FILENO, &key_pressed[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &key_pressed[1], 1) != 1)
      return '\x1b';
    if (key_pressed[0] == '[')
    {
      switch (key_pressed[1])
      {
      case 'A':
        return up;
        break;
      case 'B':
        return down;
        break;
      case 'C':
        return right;
        break;
      case 'D':
        return left;
        break;
      }
    }
  }
  return c;
}

void key_process(char key_val)
{
  switch (key_val)
  {
  case up:
  case down:
  case right:
  case left:
    move_cursor(key_val);
    break;
  case ctrl_value('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
}
void move_cursor(char key_value)
{
  switch (key_value)
  {
  case up:
    config.y_position--;
    if (config.y_position == 0)
      config.y_position++;
    break;
  case down:
    config.y_position++;
    if (config.y_position == config.rows + 1)
      config.y_position--;
    break;
  case right:
    config.x_position++;
    if (config.x_position == config.cols + 1)
      config.x_position--;
    break;
  case left:
    config.x_position--;
    if (config.x_position == 0)
      config.x_position++;
    break;
  }
  char cursor_position[32];
  snprintf(cursor_position, sizeof(cursor_position), "\x1b[%d;%dH", config.y_position, config.x_position);
  write(STDOUT_FILENO, cursor_position, strlen(cursor_position));
}
void append_string(struct initial_string *source, char *val, int len)
{
  char *append = realloc(source->val, source->length + len);
  memcpy(&append[source->length], val, len);
  source->val = append;
  source->length += len;
}

// void clear_reposition()
void initial_rendering(struct initial_string *v,int argc, char *argv[])
{
  // reset screen , the below function accepts 4 bytes where (\x1b) is 1 byte and other 3 bytes are [2J
  append_string(v, "\x1b[2J", 4);
  // TO position the cursor make us of this <esc>[1;1H where 1 and 1 represent first row and first column
  append_string(v, "\x1b[H", 3);

  if (argc > 1) 
  {
    FILE *file = fopen(argv[1],"r");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line,&linecap,file)) != -1)
    {
      while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) linelen--;
      int index = config.numrows;
      config.row = realloc(config.row, sizeof(erow) * (config.numrows + 1));
      config.row[index].size = linelen;
      config.row[index].chars = malloc(linelen + 1);
      memcpy(config.row[index].chars, line, linelen);
      config.row[index].chars[linelen] = '\0';
      config.numrows++;
    }
    free(line);
    fclose(file);
  }

  // // Adding ~ to the whole screen
  int y;
  for (y = 0; y < config.rows; y++)
  {
    if (y >= config.numrows)
    {
      append_string(v, "~", 1);
    }
    else
    {
      int len = config.row[y].size;
      if (len > config.cols) len = config.cols;
      append_string(v,config.row[y].chars ,len);
    }
    if (y < config.rows - 1)
    {
      append_string(v, "\r\n", 2);
    }
  }
  // Bring back cursor to 1x1
  append_string(v, "\x1b[H", 3);
  // Render the screen
  write(STDOUT_FILENO, v->val, v->length);
}

int main(int argc, char *argv[])
{
  // disabling default behaviour of terminal
  initial_setting();

  struct initial_string v;
  v.length = 0;
  v.val = NULL;

  initial_rendering(&v, argc, argv);

  while(1)
  {
    char key_pressed = read_key();
    key_process(key_pressed);
  }

  // Free memory
  struct initial_string *temp = &v;
  free(temp->val);

  return 0;
}