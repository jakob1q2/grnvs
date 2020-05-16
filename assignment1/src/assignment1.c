#include "arguments.h"
#include <stdio.h>

void assignment1(long start) {
    long i;
    /* Cast to void to avoid unused variable warnings. May be removed when
     * you use those variables. */
    (void)i;
    printf("%ld\n", start);
    /*====================================TODO===================================*/
    while (start != 1) {
        if (start % 2 == 0) {
            start /= 2;
            printf("%ld\n", start);
        } else {
            start = 3*start + 1;
            printf("%ld\n", start);
        }
    }
    /*===========================================================================*/
}

int main(int argc, char **argv) {
    struct arguments args;

    if (parse_args(&args, argc, argv) < 0) {
        fprintf(stderr, "Failed to parse arguments, call with "
                        "--help for more information\n");
        return -1;
    }
    assignment1(args.start);

    return 0;
}
