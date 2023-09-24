#include <stdio.h>

void unused_function(void)
{
    printf("This function used to do something useful, but is now no longer called\n");
}

int main(void)
{
    printf("We used to call 'unused_function' here.\n");
#if 0
    unused_function();
#endif
}
