#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main()
{
    char *a = malloc(10);
    strcpy(a,"shanphear");
    a[9] = '\0';
    int len = strlen(a);
    printf("%d\n",len);
    a = realloc(a,len + 2);
    memmove(&a[5],&a[4],len - 4 + 1);
    len++;
    a[4] = 'e';
    printf("%s\n",a);
    free(a);
    return 0;
}