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

typedef struct struct_erow
{
  int size;
  char *chars;
} struct_erow;

typedef struct search_txt
{
  char *chars;
  int size;
  int start_index;
  int search_index;
} search_txt;

struct search_txt s_txt;
struct terminal_config
{
  struct termios default_terminos;
  int nrows;
  int ncols;
  int x_position, y_position;
  int numrows;
  int actual_numrows;
  erow *row;
  struct_erow *actual_row;
  int row_start;
  char mode;
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
  delete,
  enter,
  end
};
enum editor_mode
{
  find,
  locked,
  edit
};
// Use this variable with fprintf function for debugging purpose
// Open output.txt to see the print statements
FILE *console_file;

void enable_default();
void initial_setting();
void append_string(struct initial_string *source, char *val, int len);
void read_file(char *filename);
void read_file_helper(int size, int start_index, char *line, int actual_index);
char read_key();
void key_process(char key_val);
void move_cursor(char key_val);
void move_search_cursor(char key_val, int m_cursor);
void set_cursor();
void update_screen(int reset);
void refresh(int m_cursor);
void edit_character(char key_value, char type);
void edit_search_text(char key_value, char type);
void initialize_first_character();

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
  config.mode = locked;
  s_txt.size = 0;
  s_txt.start_index = 0;
  s_txt.search_index = 0;
  atexit(enable_default);
}

char read_key()
{
  char c;
  read(STDIN_FILENO, &c, 1);
  char key_pressed[3];
  switch (c)
  {
  case '\x7F':
    return delete;
  case '\r':
    return enter;
  case '\x1b':
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
      case 'B':
        return down;
      case 'C':
        return right;
      case 'D':
        return left;
      }
    }
    break;
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
    if (config.mode == find) move_search_cursor(key_val,0);
    else move_cursor(key_val);
    break;
  case ctrl_value('q'):
    write(STDOUT_FILENO, "\x1b[2K", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  case ctrl_value('i'):
    config.mode = config.mode == edit ? locked : edit;
    update_screen(-1);
    break;
  case ctrl_value('f'):
    config.mode = find;
    s_txt.size = 0;
    config.y_position = config.nrows;
    config.x_position = 1;
    update_screen(-1);
    break;
  case delete:
    if (config.mode == find)
      edit_search_text('0', 'd');
    else
      edit_character('0', 'd');
    break;
  case enter:
    if (config.mode == find) edit_search_text('0', 'e');
    else edit_character('0', 'e');
    break;
  default:
    if (config.mode == find) edit_search_text(key_val, 'i');
    else edit_character(key_val, 'i');
    break;
  }
}

void edit_search_text(char key_value, char type)
{
  int a_position = config.x_position + s_txt.start_index;
  switch (type)
  {
  case 'd':
    if (a_position == 1)
      return;
    s_txt.size--;
    memmove(&s_txt.chars[a_position - 2], &s_txt.chars[a_position - 1], s_txt.size - a_position + 2);
    s_txt.chars[s_txt.size] = '\0';
    if (s_txt.start_index > 0) move_search_cursor(left,1);
    else move_search_cursor(left,0);
    break;
  case 'e':
    break;
  case 'i':
    s_txt.size++;
    s_txt.chars = realloc(s_txt.chars, s_txt.size + 1);
    s_txt.chars[s_txt.size] = '\0';
    memmove(&s_txt.chars[a_position], &s_txt.chars[a_position - 1], s_txt.size - a_position);
    s_txt.chars[a_position - 1] = key_value;
    move_search_cursor(right,0);
    break;
  default:
    break;
  }
}

void edit_character(char key_value, char type)
{
  if (config.mode == locked) return;
  if (type == 'i' && config.actual_numrows == 0) initialize_first_character();
  int m_cursor = 0;
  int y_position;
  erow row_value;
  struct_erow actual_row;
  int actual_x_postion;
  y_position = config.y_position + config.row_start;
  row_value = config.row[y_position];
  actual_row = config.actual_row[row_value.actual_index];
  actual_x_postion = config.x_position + row_value.start_index;
  if (type == 'i')
  {
    m_cursor = 1;
    if (config.x_position == config.ncols - 1) m_cursor = 2;
    config.actual_row[row_value.actual_index].chars = realloc(config.actual_row[row_value.actual_index].chars, actual_row.size + 2);
    memmove(&config.actual_row[row_value.actual_index].chars[actual_x_postion + 1], &config.actual_row[row_value.actual_index].chars[actual_x_postion], actual_row.size - actual_x_postion + 1);
    config.actual_row[row_value.actual_index].size++;
    config.actual_row[row_value.actual_index].chars[actual_x_postion] = key_value;
  }
  else if (type == 'd')
  {
    if (config.actual_numrows == 0 || (row_value.actual_index == 0 && actual_x_postion == 0)) return;
    int delete_position = actual_x_postion - 1;
    m_cursor = 3;
    if (actual_x_postion == 0)
    {
      if (config.actual_row[row_value.actual_index].size == 0)
      {
        if (config.row_start >= 1)
        {
          config.row_start--;
          move_cursor(end);
          m_cursor = 6;
        }
        memmove(&config.actual_row[row_value.actual_index], &config.actual_row[row_value.actual_index + 1], sizeof(struct_erow) * (config.actual_numrows - row_value.actual_index - 1));
        config.actual_numrows--;
      }
      else if (config.actual_row[row_value.actual_index].size > 0)
      {
        move_cursor(left);
        int new_xposition = config.row[y_position - 1].size - 1;
        int new_len = config.actual_row[row_value.actual_index - 1].size + actual_row.size;
        config.actual_row[row_value.actual_index - 1].chars = realloc(config.actual_row[row_value.actual_index - 1].chars, new_len + 1);
        config.actual_row[row_value.actual_index - 1].chars = strcat(config.actual_row[row_value.actual_index - 1].chars, actual_row.chars);
        config.actual_row[row_value.actual_index - 1].chars[new_len] = '\0';
        config.actual_row[row_value.actual_index - 1].size = new_len;
        memmove(&config.actual_row[row_value.actual_index], &config.actual_row[row_value.actual_index + 1], sizeof(struct_erow) * (config.actual_numrows - row_value.actual_index - 1));
        config.actual_numrows--;
        m_cursor = 0;
      }
    }
    else
    {
      memmove(&config.actual_row[row_value.actual_index].chars[delete_position], &config.actual_row[row_value.actual_index].chars[delete_position + 1], actual_row.size - delete_position);
      config.actual_row[row_value.actual_index].size--;
      if (config.x_position == 1 && config.actual_row[row_value.actual_index].size != 0) m_cursor = 4;
    }
  }
  else if (type == 'e')
  {
    m_cursor = 1;
    int no_f_rows = 1;
    no_f_rows = config.x_position == 0 && actual_x_postion > 0 ? 2 : 1;
    for (int c = 0; c < no_f_rows; c++)
    {
      config.actual_row = realloc(config.actual_row, sizeof(struct_erow) * (config.actual_numrows + 1));
      memmove(&config.actual_row[row_value.actual_index + 1], &config.actual_row[row_value.actual_index], sizeof(struct_erow) * (config.actual_numrows - row_value.actual_index));
      config.actual_numrows++;
    }
    if (config.x_position == 0 && actual_x_postion == 0)
    {
      config.actual_row[row_value.actual_index].chars = malloc(1);
      config.actual_row[row_value.actual_index].chars[0] = '\0';
      config.actual_row[row_value.actual_index].size = 0;
    }
    else
    {
      int temp_len = actual_row.size - actual_x_postion;
      char *temp = malloc(temp_len + 1);
      strcpy(temp, &config.actual_row[row_value.actual_index].chars[actual_x_postion]);
      config.actual_row[row_value.actual_index].chars = realloc(config.actual_row[row_value.actual_index].chars, actual_x_postion + 1);
      config.actual_row[row_value.actual_index].chars[actual_x_postion] = '\0';
      config.actual_row[row_value.actual_index].size = actual_x_postion;
      if (no_f_rows == 2)
      {
        config.actual_row[row_value.actual_index + 1].chars = malloc(1);
        config.actual_row[row_value.actual_index + 1].chars[0] = '\0';
        config.actual_row[row_value.actual_index + 1].size = 0;
      }
      config.actual_row[row_value.actual_index + no_f_rows].chars = malloc(temp_len + 1);
      memcpy(config.actual_row[row_value.actual_index + no_f_rows].chars, temp, temp_len);
      config.actual_row[row_value.actual_index + no_f_rows].chars[temp_len] = '\0';
      config.actual_row[row_value.actual_index + no_f_rows].size = temp_len;
    }
  }
  refresh(m_cursor);
}

void move_search_cursor(char key_val, int m_cursor)
{
  int a_position = config.x_position + s_txt.start_index;
  switch (key_val)
  {
  case left:
    if (a_position == 1) return;
    if((config.x_position == 1 && a_position > 1) || m_cursor == 1) s_txt.start_index--;
    else config.x_position--;
    update_screen(0);
    break;
  case right:
    if (a_position > s_txt.size) return;
    if (config.x_position > config.ncols - 20) s_txt.start_index++;
    else config.x_position++;
    update_screen(0);
    break;
  }
  set_cursor();
}

void move_cursor(char key_value)
{
  int y_p = config.y_position + config.row_start;
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
    y_p = config.y_position + config.row_start;
    if (config.x_position > config.row[y_p].size) config.x_position = config.row[y_p].size;
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
    else if ((config.y_position + 1 + config.row_start) < config.numrows)
    {
      config.y_position++;
    }
    y_p = config.y_position + config.row_start;
    if (config.x_position > config.row[y_p].size) config.x_position = config.row[y_p].size;
    break;
  case right:
    y_p = config.y_position + config.row_start;
    if (config.x_position + 1 == config.row[y_p].size + 1)
    {
      if (y_p + 1 < config.numrows)
      {
        config.x_position = 0;
        move_cursor(down);
        return;
      }
    }
    if (config.x_position + 1 <= config.row[y_p].size) config.x_position++;
    break;
  case left:
    y_p = config.y_position + config.row_start;
    if (config.x_position - 1 == -1)
    {
      if (y_p - 1 >= 0)
      {
        config.x_position = config.row[y_p - 1].size;
        move_cursor(up);
        return;
      }
    }
    if (config.x_position - 1 >= 0) config.x_position--;
    break;
  case end:
    y_p = config.y_position + config.row_start;
    config.x_position = config.row[y_p].size;
    break;
  }
  set_cursor();
}

void set_cursor()
{
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

void read_file(char *filename)
{
  FILE *file = fopen(filename, "r");
  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  while ((linelen = getline(&line, &linecap, file)) != -1)
  {
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) linelen--;
    int temp_len = linelen;
    int start_index = 0;
    // File data and file indexing
    config.actual_row = realloc(config.actual_row, sizeof(struct_erow) * (config.actual_numrows + 1));
    config.actual_row[config.actual_numrows].size = temp_len;
    config.actual_row[config.actual_numrows].chars = malloc(temp_len + 1);
    memcpy(config.actual_row[config.actual_numrows].chars, line, temp_len);
    config.actual_row[config.actual_numrows].chars[temp_len] = '\0';
    while (temp_len >= config.ncols - 1)
    {
      read_file_helper(config.ncols - 1, start_index, line, config.actual_numrows);
      temp_len = temp_len - config.ncols + 1;
      start_index++;
    }
    if (temp_len > 0 || (temp_len == 0 && config.actual_row[config.actual_numrows].size == 0))
    {
      read_file_helper(temp_len, start_index, line, config.actual_numrows);
    }
    config.actual_numrows++;
  }
  free(line);
  fclose(file);
}

void read_file_helper(int size, int start_index, char *line, int actual_index)
{
  int index = config.numrows;
  config.row = realloc(config.row, sizeof(erow) * (config.numrows + 1));
  config.row[index].size = size;
  config.row[index].chars = malloc(size + 1);
  int start_index_v = start_index * (config.ncols - 1);
  config.row[index].start_index = start_index_v;
  config.row[index].actual_index = actual_index;
  memcpy(config.row[index].chars, &line[start_index_v], size);
  config.row[index].chars[size] = '\0';
  config.numrows++;
}

void refresh(int m_cursor)
{
  struct terminal_config *temp_config = &config;
  for (int i = 0; i < config.numrows; i++)
  {
    free(temp_config->row[i].chars);
  }
  config.numrows = 0;
  for (int i = 0; i < config.actual_numrows; i++)
  {
    int temp_len = config.actual_row[i].size;
    int start_index = 0;
    while (temp_len >= config.ncols - 1)
    {
      read_file_helper(config.ncols - 1, start_index, config.actual_row[i].chars, i);
      temp_len = temp_len - config.ncols + 1;
      start_index++;
    }
    if (temp_len > 0 || (temp_len == 0 && config.actual_row[i].size == 0))
    {
      read_file_helper(temp_len, start_index, config.actual_row[i].chars, i);
    }
  }
  if (m_cursor == -1) return;
  update_screen(0);
  switch (m_cursor)
  {
  case 1:
    move_cursor(right);
    break;
  case 2:
    move_cursor(right);
    move_cursor(right);
    break;
  case 3:
    move_cursor(left);
    break;
  case 4:
    move_cursor(left);
    move_cursor(left);
    break;
  case 5:
    move_cursor(up);
    break;
  case 6:
    move_cursor(end);
  default:
    set_cursor();
    break;
  }
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
    else if (index < config.numrows && config.row[index].size > 0)
    {
      int length = config.row[index].size;
      append_string(&v, config.row[index].chars, length);
    }
    // Append this after each line
    append_string(&v, "\x1b[K", 3);

    if (y < config.nrows - 1)
    {
      append_string(&v, "\r\n", 2);
    }
  }
  append_string(&v, "\r\n", 2);
  append_string(&v, "\x1b[1;31m", 7);
  if (config.mode == locked)
  {
    append_string(&v, ":Locked", 7);
  }
  else if (config.mode == edit)
  {
    append_string(&v, ":Edit Mode", 10);
  }
  else if (config.mode == find)
  {
    if (reset == -1) append_string(&v, ":Enter the text to be searched", 30);
    else
    {
      append_string(&v, ":", 1);
      append_string(&v, &s_txt.chars[s_txt.start_index], s_txt.size < config.ncols - 19 ? s_txt.size + 1 : config.ncols - 20);
    }
  }
  append_string(&v, "\x1b[0;39m", 7);
  append_string(&v, "\x1b[K", 3);
  // Bring back cursor to 1x1
  if (reset == 1) append_string(&v, "\x1b[H", 3);
  // Render the screen
  write(STDOUT_FILENO, v.val, v.length);
  if (reset == -1) set_cursor();
  struct initial_string *temp = &v;
  free(temp->val);
}
void initialize_first_character()
{
  config.actual_row = realloc(config.actual_row, sizeof(struct_erow) * (config.actual_numrows + 1));
  config.actual_row[0].chars = malloc(1);
  config.actual_row[0].chars[0] = '\0';
  config.actual_row[0].size = 0;
  config.actual_numrows++;
  refresh(-1);
}

void show_instructions()
{
  char *row[14];
  row[0] = "\x1b[1;34m    ____  _____      __________  ______________  ____ ";
  row[1] = "   / __ \\/ ___/     / ____/ __ \\/  _/_  __/ __ \\/ __ \\";
  row[2] = "  / /_/ /\\__ \\     / __/ / / / // /  / / / / / / /_/ /";
  row[3] = " / _, _/___/ /    / /___/ /_/ // /  / / / /_/ / _, _/";
  row[4] = "/_/ |_|/____/    /_____/_____/___/ /_/  \\____/_/ |_|\x1b[0;39m";
  row[5] = "";
  row[6] = "";
  row[7] = "Ctrl + I - Toggle between Edit and Locked Mode";
  row[8] = "Ctrl + Q - Quit Editor";
  row[9] = "Ctrl + S - Save Changes";
  row[10] = "Ctrl + F - Search";
  row[11] = "";
  row[12] = "";
  row[13] = "\x1b[0;104mPress ENTER key to continue\x1b[0;39m";
  struct initial_string instruction;
  instruction.length = 0;
  instruction.val = NULL;
  append_string(&instruction, "\x1b[2K", 4);
  append_string(&instruction, "\x1b[H", 3);
  for (int i = 0; i < 14; i++)
  {
    int char_len = strlen(row[i]);
    if (char_len != 0) append_string(&instruction, row[i], char_len);
    append_string(&instruction, "\x1b[K", 3);
    append_string(&instruction, "\r\n", 2);
  }
  write(STDOUT_FILENO, instruction.val, instruction.length);
  char c;
  while (c != '\r') read(STDIN_FILENO, &c, 1);
}

int main(int argc, char *argv[])
{
  console_file = fopen("output.txt", "w");
  // disabling default behaviour of terminal
  initial_setting();
  show_instructions();
  // If file name is passed
  if (argc > 1)
  {
    config.mode = locked;
    read_file(argv[1]);
  }
  else
  {
    config.mode = edit;
    initialize_first_character();
  }
  update_screen(1);
  while (1)
  {
    char key_pressed = read_key();
    key_process(key_pressed);
  }

  // Free memory
  fclose(console_file);
  struct initial_string *temp = &v;
  free(temp->val);
  struct terminal_config *temp_config = &config;
  free(s_txt.chars);
  for (int i = 0; i < config.numrows; i++)
  {
    free(temp_config->row[i].chars);
  }
  for (int i = 0; i < config.actual_numrows; i++)
  {
    free(temp_config->actual_row[i].chars);
  }

  return 0;
}