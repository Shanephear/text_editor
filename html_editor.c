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
  int nrows;
  int ncols;
  int x_position, y_position;
  int numrows;
  erow *row;
  int row_start;
  int col_start;
};

struct terminal_config config;

struct initial_string
{
  char *val;
  int length;
};
struct initial_string v;
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
void read_file(char *filename);
char read_key();
void key_process(char key_val);
void move_cursor(char key_val);
void update_screen(int reset);

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
  config.nrows = ws.ws_row - 1;
  config.ncols = ws.ws_col;
  config.x_position = 0;
  config.y_position = 0;
  config.numrows = 0;
  config.row_start = 0;
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
    write(STDOUT_FILENO, "\x1b[2K", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  default:
    break;
  }
}
void move_cursor(char key_value)
{
  switch (key_value)
  {
  case up:
    if (config.y_position - 1 == -1)
    {
      if (config.numrows > config.nrows && config.row_start >= 1)
      {
        config.row_start--;
        update_screen(0);
      }
    }
    else config.y_position--;
    break;
  case down:
    if ((config.y_position + 1) == config.nrows)
    {
      if (config.numrows > config.nrows && config.row_start < (config.numrows - config.nrows))
      {
        config.row_start++;
        update_screen(0);
      }
    }
    else config.y_position++;
    break;
  case right:
    config.x_position++;
    if (config.x_position == config.ncols + 1)
      config.x_position--;
    break;
  case left:
    config.x_position--;
    if (config.x_position == -1)
      config.x_position++;
    break;
  }
  char cursor_position[32];
  snprintf(cursor_position, sizeof(cursor_position), "\x1b[%d;%dH", config.y_position + 1, config.x_position + 1);
  write(STDOUT_FILENO, cursor_position, strlen(cursor_position));
}
void append_string(struct initial_string *source, char *val, int len)
{
  char *append = realloc(source->val, source->length + len);
  if (append == NULL) return;
  memcpy(&append[source->length], val, len);
  source->val = append;
  source->length += len;
}

// void clear_reposition()
void read_file(char *filename)
{
    FILE *file = fopen(filename,"r");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line,&linecap,file)) != -1)
    {
      while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) linelen--;
      int temp_len = linelen;
      int start_index = 0;
      while(temp_len >= config.ncols) 
      {
        int index = config.numrows;
        config.row = realloc(config.row, sizeof(erow) * (config.numrows + 1));
        config.row[index].size = config.ncols;
        config.row[index].chars = malloc(config.ncols + 1);
        int start_index_v = start_index * (config.ncols - 1);
        memcpy(config.row[index].chars, &line[start_index_v], config.ncols);
        config.row[index].chars[config.ncols] = '\0';
        temp_len -= config.ncols;
        start_index++;
        config.numrows++;
      }
      if (temp_len > 0)
      {
        int index = config.numrows;
        config.row = realloc(config.row, sizeof(erow) * (config.numrows + 1));
        config.row[index].size = temp_len;
        config.row[index].chars = malloc(temp_len + 1);
        int start_index_v = start_index * config.ncols;
        memcpy(config.row[index].chars, &line[start_index_v], config.ncols);
        config.row[index].chars[temp_len] = '\0';
        config.numrows++;
      }
    }
    free(line);
    fclose(file);
}

void update_screen(int reset)
{
  v.length = 0;
  v.val = NULL;
  // To clear the screen
  append_string(&v, "\x1b[2K", 4);
  // TO position the cursor make us of this <esc>[1;1H where 1 and 1 represent first row and first column
  append_string(&v, "\x1b[H", 3);

  // Adding ~ and read file into the screen
  int y;
  for (y = 0; y < config.nrows; y++)
  {
    int index = y + config.row_start;
    if (y >= config.numrows)
    {
      append_string(&v, "~", 1);
    }
    else
    {
      int length = config.row[index].size;
      append_string(&v,config.row[index].chars ,length);
    }
    
    //Append this after each line
    append_string(&v, "\x1b[K", 3);

    if (y < config.nrows - 1)
    {
      append_string(&v, "\r\n", 2);
    }
  }
  if (reset == 1)
  {
    // Bring back cursor to 1x1
    append_string(&v, "\x1b[H", 3);
  }
  // Render the screen
  write(STDOUT_FILENO, v.val, v.length);
  struct initial_string *temp = &v;
  free(temp->val);
}

int main(int argc, char *argv[])
{
  // disabling default behaviour of terminal
  initial_setting();

  // If file name is passed
  if (argc > 1)
  {
    read_file(argv[1]);
  }
  update_screen(1);
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