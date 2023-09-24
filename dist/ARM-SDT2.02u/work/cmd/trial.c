/* trial.c - This is a short demonstration program for use     */
/* with armsd. The program serves no real function, but simply */
/* acts as an example which can be used to set watchpoints     */
/* and breakpoints on.                                         */
/* The initmem function is used to clear a block of memory     */
/* The quickfunc function is a very simple function            */
/* The results function is a demonstration of semi-hosting.    */

#include <stdio.h>

#define BLOCKSTART (int *) 0x4000
#define BLOCKEND   (int *) 0x4100
#define TESTADDR   (int *) 0x4200


/* Memory Initialisation to Zero */


static void initmem()
{
int *i;
int a;

a = 0;
i=BLOCKSTART;

do {
	*i++ = 0;
    a++;
} while (i<BLOCKEND);

}

static void results()
{
printf("The memory area has now been cleared.\n");
}

static void quickfunc(int a, int b, int c)
{
int x,y,z;
x = 3;
y = 4;
z = x * y;
x = (z << 8) + y;
}

int main()
{
  int *datablock;
  int a,b,c;

datablock = BLOCKSTART;
a = 10;
b = 20;
c = 30;

initmem();
quickfunc(a,b,c);
quickfunc(b,c,a);
quickfunc(c,a,b);
results();


}
