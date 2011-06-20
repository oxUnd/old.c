#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char c;
    scanf("%s", &c);
    switch (c) {
        case 'a' ... 'z':
            printf("Lower-case: %c\n", c);
            break;
        case 'A' ... 'Z':
            printf("Upper-case: %c\n", c);
            break;
        case '1' ... '9':
            printf("number: %c\n", c);
            break;
        default:
            printf("Error!\n");
            break;
    }
    return 0;
}
