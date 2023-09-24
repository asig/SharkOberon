#include <stdio.h>

/* Declare subroutine before used by main */
void subroutine (void);

int main() {
        printf("Hello World from main\n");
        subroutine();
        printf("And Goodbye from main\n");
        return 0;
}

/* Define subroutine */
void subroutine() {
        printf("Hello from subroutine\n");
}
