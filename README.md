<img width="700" alt="Screenshot 2024-02-04 at 11 43 04 PM" src="https://github.com/Shanephear/text_editor/assets/67944253/6d4e9147-6eb6-45be-b73c-07bb21100eef">

# RS Editor: Text Editor

This is a text editor built using c program named as RS Editor. The program can edit an existing file or create a new file.
# RS Editor
#### Video Demo:  <URL HERE>
#### Description:
Steps to be followed to run the program:

```
make text_editor
./text_editor
```
## text_editor.c
The program is written in such a way it restricts the normal behavior of a terminal.

The design steps are as follows:

### 1. Reconfiguring the terminal
The terminal is reconfigured with the help of ***terminos.h*** library to restrict default behavior from cursor movement to printing. It also blocks certain inputs.A specific amount of space is allotted based on the window size for the editor. When the program exits , these configurations are reverted.

### 2.Configuring required Inputs
<kbd>CTRL</kbd> + <kbd>q</kbd> => To exit the program and reset terminal to default configuration.

<kbd>CTRL</kbd> + <kbd>i</kbd> => To switch between edit mode and locked mode

* Edit mode - The editor allows to edit text.

* Locked mode - The editor does not allow editing.

<kbd>CTRL</kbd> + <kbd>f</kbd> => To enter Find mode.

* Find mode - The editor allows to search for text and pressing enter key after entering the text to be searched will show each occurrence by moving the cursor to the start of the search text.

<kbd>CTRL</kbd> + <kbd>s</kbd> => To save the file and will prompt for the file name if it's a new file.

<kbd>Up</kbd>, <kbd>down</kbd>, <kbd>left</kbd>, <kbd>right</kbd> arrow keys => To move around the editor.

<kbd>delete</kbd> key => To delete characters from right to left.

<kbd>return</kbd> key => When in edit mode, will insert a new line. While in Find mode will show each occurrence of the search text.

Other Characters => Every other character will be added to the file.

### 3.Status Bar
A status bar is available at the bottom to show what mode the editor is currently in (Edit/locked). It also accepts input while in Find mode and when a new file is created and is saved, it prompts for the name of the file.
### 4.To debug while development
As the terminal’s default behavior is restricted, print function in C cannot be used for debugging purpose. To overcome this console_file.txt is created and using fprintf function anything can be printed to the ***console_file.txt***.

#### The functions that are available in ***text_editor.c*** are as follows:
`starter()`&nbsp;&nbsp;Clears the terminal, prepares the debugging file, reconfigures the terminal to editor setting and show the instruction page.

`open_editor_f(char * filename)`&nbsp;&nbsp;Opens editor with existing file.

`open_editor()`&nbsp;&nbsp;Opens a blank editor.

`read_file(filename)`&nbsp;&nbsp;Read the file based on the path and stores data in two formats. One is file format and the other is the editor format.

`read_file_helper(int size, int start_index, char *line, int actual_index)`&nbsp;&nbsp;Helper function for reading the file

`initialize_first_character()`&nbsp;&nbsp;Setups the file format and editor format variables when it’s a blank editor.

`editor()`&nbsp;&nbsp;Starts up the editor, read every inputs(Key press).

`read_key()`&nbsp;&nbsp;Identifies the enter, delete and arrow keys.

`key_process(char key_val)`&nbsp;&nbsp;Configuring what should be done based on the inputs.

`enable_default()`&nbsp;&nbsp;Resets the terminal to default setting.

`initial_setting()`&nbsp;&nbsp;Sets up editor configuration for the terminal.

`status_bar_initial()`&nbsp;&nbsp;Initializes/Resets the variables used for status bar to initial setting.

`append_string(struct initial_string *source, char *val, int len)`&nbsp;&nbsp;Appends source string to destination string.

`move_cursor(char key_val)`&nbsp;&nbsp;Moves the cursor around the editor area.

`move_status_cursor(char key_val, int m_cursor)`&nbsp;&nbsp;Moves the cursor around the status bar area.

`set_cursor()`&nbsp;&nbsp;Updates the cursor location.

`update_screen(int reset)`&nbsp;&nbsp;Updates the screen to reflect key presses/inputs.

`edit_character(char key_value, char type)`&nbsp;&nbsp;Performs inserting, deleting and adding new line in editor area.

`refresh(int m_cursor)`&nbsp;&nbsp;This is a helper function for edit_character function as this places the cursor in the right place after edit_character function’s functionalities.

`edit_status_text(char key_value, char type)`&nbsp;&nbsp;Performs inserting and deleting in status bar area.

`save_changes()`&nbsp;&nbsp;Save changes to the existing file or new file that is created by the editor.

`search()`&nbsp;&nbsp;Performs the search operation.

`move_cursor_to_status()`&nbsp;&nbsp;Moves the cursor to status bar.

`free_memory()`&nbsp;&nbsp;Frees up all the allocated memory.

Improvements that can be made to ***text_editor.c*** : Functions can be categorized based on functionalities and split up into different files.

## text_editor.h
This file consists of functions from ***text_editor.c*** file. Only the functions that are required to start up the editor needs to be added to this file. This file is used for importing these functions to ***starter.c***

## starter.c
This is the starter file of the program. This is a straightforward program that shows the editor name, version, author and prompts if an existing file needs to be opened.

The design steps are as follows:

1. Type of editor to be opened
* Prompt if an existing file is to be opened or not with an yes or no question.

2. If Yes
* Call `open_editor_f(filename)`&nbsp; function from ***text_editor.h*** to open the editor and show the data available in existing file.If the file is not present show error message and exit the program.

3. If No
* Call `open_editor()`&nbsp; function from ***text_editor.h*** to open blank editor.

## Makefile
Makefile builds the ***text_editor.c*** and ***starter.c*** .It produces the ***text_editor*** file which opens the text editor.














