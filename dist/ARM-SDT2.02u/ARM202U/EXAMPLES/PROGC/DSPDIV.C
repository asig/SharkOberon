#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  if (argc != 3)
    puts("needs 2 numeric args");
  else
  {
    __sdiv32by16 result;

    result = __rt_sdiv32by16(atoi(argv[1]), atoi(argv[2]));

    printf("quotient %d\n", result.quot);
    printf("remainder %d\n", result.rem);
  }
  return 0;
}
