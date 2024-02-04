#include<stdio.h>
#include "text_editor.h"
#include<string.h>

int main()
{
    char c;
    char filename[256];
    printf("\x1b[1;34mRS Editor 1.0\n");
    printf("-- created by Shanephear\n\n");
    printf("\x1b[0;39mDo you want to open an existing file <Y/N>?");
    scanf(" %c",&c);
    if (c == 'Y' || c == 'y')
    {
        printf("Enter the path of the file: ");
        scanf("%255s",filename);
        FILE* file = fopen(filename,"r");
        if (file)
        {
            fclose(file);
            starter();
            open_editor_f(filename);
        }
        else printf("\x1b[1;31mFile not found!\n\x1b[0;39m");
    }
    else 
    {
        starter();
        open_editor();
    }
    return 0;
}