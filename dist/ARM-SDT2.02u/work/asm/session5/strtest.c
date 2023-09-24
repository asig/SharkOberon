#include <stdio.h>
extern void strcopy(char *d, char *s);

int main()
{    char *srcstr = "First string - source ";
     char *dststr = "Second string - destination ";
 
     printf("Before copying:\n");
     printf("  %s\n  %s\n",srcstr,dststr);
     strcopy(dststr,srcstr);
     printf("After copying:\n");
     printf("  %s\n  %s\n",srcstr,dststr);
     return (0);
}
