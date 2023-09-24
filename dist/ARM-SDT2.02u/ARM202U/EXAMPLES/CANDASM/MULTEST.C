/* Demonstrate mul64 */

#include <stdio.h>
#include "int64.h"
#include "mul64.h"

int main()
{ int64 res;
  unsigned a,b;

  printf( "Enter two unsigned 32-bit numbers in hex eg.(100 FF43D)\n" );
  if( scanf( "%x %x", &a, &b ) != 2 )
  { puts( "Bad numbers" );
  } else
  { res=mul64(a,b);
    printf( "Least significant word of result is %8X\n", res.lo );
    printf( "Most  significant word of result is %8X\n", res.hi );
  }
  return( 0 );
}
