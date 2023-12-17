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
  int actual_index;
  int start_index;
} erow;

typedef struct actual_erow
{
  int size;
  char *chars;
} actual_erow;

struct terminal_config
{
  struct termios default_terminos;
  int nrows;
  int ncols;
  int x_position, y_position;
  int numrows;
  int actual_numrows;
  erow *row;
  actual_erow *actual_row;
  int row_start;
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
  down,
  delete
};
// Use this variable with fprintf function for debugging purpose
// Open output.txt to see the print statements
FILE *console_file;


void enable_default();
void initial_setting();
void append_string(struct initial_string *source, char *val, int len);
void read_file(char *filename);
void read_file_helper(int actual_index);
char read_key();
void key_process(char key_val);
void move_cursor(char key_val);
void update_screen(int reset);
void insert_character(char key_value);
void delete_character();


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
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  config.nrows = ws.ws_row - 1;
  config.ncols = ws.ws_col;
  config.x_position = 0;
  config.y_position = 0;
  config.numrows = 0;
  config.actual_numrows = 0;
  config.row_start = 0;
  atexit(enable_default);
}

char read_key()
{
  char c;
  read(STDIN_FILENO, &c, 1);
  if (c == '\x7F')
  {
    return delete;
  }
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
    insert_character(key_val);
    break;
  case ctrl_value('q'):
    write(STDOUT_FILENO, "\x1b[2K", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  case delete:
    delete_character();
    break;
  default:
    break;
  }
}

void insert_character(char key_value)
{
  int y_position = config.y_position + config.row_start;
  erow row_value = config.row[y_position];
  actual_erow actual_row = config.actual_row[row_value.actual_index];
  int actual_x_postion = config.x_position + row_value.start_index;
  fprintf(console_file,"x:%d ,y:%d ,actual index: %d",config.x_position,y_position,row_value.actual_index);
  fprintf(console_file,"value:%c ,actual_value: %c\n",row_value.chars[config.x_position],actual_row.chars[actual_x_postion]);
}

void delete_character()
{

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
      config.actual_row = realloc(config.actual_row, sizeof(actual_erow)*(config.actual_numrows + 1));
      config.actual_row[config.actual_numrows].size = linelen;
      config.actual_row[config.actual_numrows].chars = malloc(linelen + 1);
      memcpy(config.actual_row[config.actual_numrows].chars, line, linelen);
      config.actual_row[config.actual_numrows].chars[linelen] = '\0';
      while(temp_len >= config.ncols)
      {
        int index = config.numrows;
        config.row = realloc(config.row, sizeof(erow) * (config.numrows + 1));
        config.row[index].size = config.ncols;
        config.row[index].chars = malloc(config.ncols + 1);
        int start_index_v = start_index * (config.ncols - 1);
        config.row[index].actual_index = config.actual_numrows;
        config.row[index].start_index = start_index_v;
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
        int start_index_v = start_index * (config.ncols - 1);
        config.row[index].actual_index = config.actual_numrows;
        config.row[index].start_index = start_index_v;
        memcpy(config.row[index].chars, &line[start_index_v], temp_len);
        config.row[index].chars[temp_len] = '\0';
        config.numrows++;
      }
      config.actual_numrows++;
    }
    free(line);
    fclose(file);
}

void read_file_helper(int actual_index)
{
  // int index = config.numrows;
  // config.row = realloc(config.row, sizeof(erow) * (config.numrows + 1));
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
  console_file = fopen("output.txt", "w");
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
  fclose(console_file);
  struct initial_string *temp = &v;
  free(temp->val);
  struct terminal_config *temp_config = &config;
  for (int i = 0; i < config.numrows;i++)
  {
    free(temp_config->row[i].chars);
  }
  for (int i = 0; i < config.actual_numrows;i++)
  {
    free(temp_config->actual_row[i].chars);
  }

  return 0;
}