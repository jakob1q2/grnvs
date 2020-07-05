#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

uint64_t getMac(uint8_t *buf, uint8_t offset) {

    uint64_t res = buf[offset];
    return res;
}

int main(int argc, char const *argv[]) {
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint64_t MULTI_MASK = 0x100000;
    uint64_t x = 0xe00000;
    uint8_t buf[10] = {0xff, 0x43, 0x22, 0x56, 0x32, 0x13, 0x4, 0x3, 0x3, 0x3};

    x = x & MULTI_MASK;
    if (x) {
        printf("multi\n");
    } else {
        printf("single\n");
    }
    fprintf(stdout, "x is 0x%02x\n", x);
    uint64_t mac = getMac(buf, 0);
    fprintf(stdout, "Mac is 0x%x\n", mac);
    fprintf(stdout, "exp %f\n", 1.0e3);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double totalTime = ((double)end.tv_sec + 1.0e-9 * end.tv_nsec) -
                       ((double)start.tv_sec + 1.0e-9 * start.tv_nsec);
    uint32_t totalByte = 2000;
    fprintf(stdout, "parseByte %f\n", (double)totalByte);
    fprintf(stdout, "rate %f\n", ((double)totalByte) / totalTime / 1.0e3);
    return 0;
}
