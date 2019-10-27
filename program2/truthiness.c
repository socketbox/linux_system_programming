#include <stdio.h>

int main()
{
  char n = '\0';
  int *ptr = NULL;
  char *zero = NULL;
  char z = NULL;
  char y = '\0';

  printf("\\0 == \\0? %i\n", n == y);
  printf("\\0 == \\0? %i\n", n == y);
  printf("\\0 == NULL char ptr? %i\n", n == zero);
  printf("\\0 == NULL int ptr? %i\n", n == ptr);
  printf("\\0 == NULL primitive? %i\n", n == z);
  
  printf("Is \\0 == 0? "); 
  if('\0' == 0)
    printf("Yes\n");
  else
    printf("No\n");

  printf("Is \\0 true? "); 
  if('\0')
    printf("Yes\n");
  else
    printf("No\n");

    unsigned int range = 6;
    int nrange = (-range);
    int div = nrange/range;
    printf("nrange: %i\n", nrange);
    printf("div: %i\n", div);
}
