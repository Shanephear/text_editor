#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// typedef struct actual_erow
// {
//   int size;
//   char *chars;
// } actual_erow;

// // int main()
// // {
// //     char *a = malloc(10);
// //     strcpy(a,"shanphear");
// //     a[9] = '\0';
// //     int len = strlen(a);
// //     printf("%d\n",len);
// //     a = realloc(a,len + 2);
// //     memmove(&a[5],&a[4],len - 4 + 1);
// //     len++;
// //     a[4] = 'e';
// //     printf("%s\n",a);
// //     free(a);
// //     return 0;
// // }

// int main()
// {
//     char *a = malloc(11);
//     char *b = malloc(11);
//     strcpy(a,"shanephear\0");
//     strcpy(b,"shanephear\0");
//     int slice_position = 4;
//     int temp_len = strlen(b);
//     char *temp = malloc(temp_len + 1);
//     strcpy(temp,b);
//     temp[temp_len] = '\0';
//     int new_len = strlen(a) - (slice_position + 1) + temp_len;
//     printf("%d\n",new_len);
//     b = realloc(b,new_len + 1);
//     strcpy(b,&a[slice_position + 1]);
//     strcat(b,temp);
//     b[new_len] = '\0';
//     a = realloc(a,slice_position + 2);
//     a[slice_position + 1] = '\0';
//     printf("%s\n",b);
//     printf("%s\n",a);
//     free(a);
//     free(b);
//     free(temp);
//     return 0;
// }

// int main()
// {
//     int len = 11;
//     char *a = malloc(len);
//     strcpy(a,"shanephear\0");
//     a = realloc(a,len - 1);
//     memmove(&a[4],&a[5], 5);
//     a[len - 2] = '\0';
//     printf("%s\n",a);
//     // printf("%c\n",a[len+1]);
//     free(a);
// }
// int main() {
//   char haystack[] = "Hello, world world!";
//   char needle[] = "world";
//   char *ptr;

//   ptr = strstr(&haystack[8], needle);
//   if (ptr != NULL) {
//     printf("The substring \"%s\" was found at position %ld.\n", needle, ptr - haystack);
//   } else {
//     printf("The substring \"%s\" was not found.\n", needle);
//   }

//   return 0;
// }

int main()
{
  int float_vale = 10;
  printf("%d\n",(int)floor(10.5));
  return 0;
}