#include <stdio.h>

extern void subroutine(void);

int main() {
        printf("Hello World from main\n");
        subroutine();
        printf("And Goodbye from main\n");
        return 0;
}

