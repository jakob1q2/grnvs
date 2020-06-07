#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "arguments.h"
#include "checksums.h"
#include "hexdump.h"
#include "math.h"
#include "raw.h"

/*====================================TODO===================================*/
/* Put your required struct definitions */
struct typeInfo {
    u_int16_t ethType;
    u_int32_t frameCounter;
    u_int32_t byteCounter;
    struct typeInfo *next;
};

uint8_t mymac[ETH_ALEN];

/* Put your ancillary functions here*/
struct typeInfo *newInfo(u_int16_t type) {
    struct typeInfo *ti = malloc(sizeof(struct typeInfo));
    if (ti != NULL) {
        ti->ethType = type;
        ti->frameCounter = 0;
        ti->byteCounter = 0;
        ti->next = NULL;
    }

    return ti;
}

struct typeInfo *getInfoByType(u_int16_t ethType, struct typeInfo *head) {
    struct typeInfo *temp = head;
    while (temp != NULL && temp->ethType != ethType) {
        temp = temp->next;
    }
    return temp;
}

void freeInfo(struct typeInfo *head) {
    struct typeInfo *temp = head;
    while (head != NULL) {
        head = head->next;
        free(temp);
        temp = head;
    }
}

struct typeInfo *insertInfo(struct typeInfo *new, struct typeInfo *head) {
    if (head == NULL) {
        head = new;
        return head;
    }

    struct typeInfo *temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new;
    return head;
}

struct typeInfo *removeByType(uint16_t type, struct typeInfo *head) {
    if (head == NULL) {
        return head;
    }

    struct typeInfo *temp = head;
    if (head->ethType == type) {
        head = head->next;
        free(temp);
        return head;
    }

    while (temp->next != NULL && temp->next->ethType != type) {
        temp = temp->next;
    }
    if (temp->next == NULL) {
        return head;
    } else {
        struct typeInfo *t = temp->next;
        temp->next = t->next;
        free(t);
        return head;
    }
}

struct typeInfo *min(struct typeInfo *head) {

    struct typeInfo *cur = head;
    struct typeInfo *temp = head;
    while (temp != NULL) {
        if (temp->ethType < cur->ethType) {
            cur = temp;
        }
        temp = temp->next;
    }
    return cur;
}

int isMulticast(u_int8_t mac_first) {
    if (mac_first & 0x1) {
        return 1;
    } else {
        return 0;
    }
}

void getMac(uint8_t *src, uint8_t offset, uint8_t *dest) {
    memcpy(dest, src + offset, ETH_ALEN);
}

int macsEqual(uint8_t *x, uint8_t *y) {
    for (int i = 0; i < ETH_ALEN; i++) {
        if (x[i] != y[i]) {
            return 0;
        }
    }
    return 1;
}

uint64_t getType(uint8_t *buf) {
    uint8_t type[2];
    memcpy(type, buf + (2 * ETH_ALEN), 2);
    uint64_t res = type[0];
    res <<= 8;
    res += type[1];
    return res;
}

/*===========================================================================*/

void assignment2(int fd, int frames) {
    unsigned int timeout = 10000;
    uint8_t recbuffer[1514];
    size_t ret;

    /*====================================TODO===================================*/
    /* If you want to set up any data/counters before the receive loop,
     * this is the right location
     */
    uint32_t myFrames = 0;
    uint32_t multiCastFrames = 0;
    struct typeInfo *infoList = NULL;
    struct typeInfo *temp = NULL;
    uint16_t eType = 0;
    uint8_t mac[ETH_ALEN] = {0};
    struct timespec start;
    struct timespec end;
    uint32_t totalByte = 0;
    double totalTime = 0.0;

    memcpy(&mymac, grnvs_get_hwaddr(fd), ETH_ALEN);

    /*===========================================================================*/

    /* This is the ready marker! do not remove! */
    fprintf(stdout, "I am ready!\n");

    /*====================================TODO===================================*/
    /* Update the loop condition */
    for (int i = 0; i < frames; i++) {
        /*===========================================================================*/

        clock_gettime(CLOCK_MONOTONIC, &start);
        ret = grnvs_read(fd, recbuffer, sizeof(recbuffer), &timeout);
        clock_gettime(CLOCK_MONOTONIC, &end);
        totalTime += ((double)end.tv_sec + 1.0e-9 * end.tv_nsec) -
                     ((double)start.tv_sec + 1.0e-9 * start.tv_nsec);
        if (ret == 0) {
            fprintf(stderr, "Timed out, this means there was nothing to "
                            "receive. Do you have a sender set up?\n");
            break;
        }
        /*====================================TODO===================================*/
        /* This is the receive loop, 'recbuffer' will contain the received
         * frame. 'ret' tells you the length of what you received.
         * Anything that should be done with every frame that's received
         * should be done here.
         */
        eType = getType(recbuffer);
        temp = getInfoByType(eType, infoList);
        getMac(recbuffer, 0, mac);
        if (temp == NULL) {
            temp = newInfo(eType);
            temp->frameCounter = 1;
            temp->byteCounter = ret;
            infoList = insertInfo(temp, infoList);
        } else {
            temp->frameCounter += 1;
            temp->byteCounter += ret;
        }
        if (macsEqual(mymac, mac)) {
            myFrames += 1;
        }
        if (isMulticast(mac[0])) {
            multiCastFrames += 1;
        }
        totalByte += ret;

        /*===========================================================================*/
    }

    /*====================================TODO===================================*/
    /* Print your summary here */
    while (infoList != NULL) {
        temp = min(infoList);
        fprintf(stdout, "0x%04x: %d frames, %d bytes\n", temp->ethType,
                temp->frameCounter, temp->byteCounter);
        infoList = removeByType(temp->ethType, infoList);
    }
    fprintf(stdout, "%d of them were for me\n", myFrames);
    fprintf(stdout, "%d of them were multicast\n", multiCastFrames);
    fprintf(stdout,
            "Total rate %07f Mbit/s | %07f MiB/s | %07f kB/s | %07f Kibit/s\n",
            ((double)totalByte) / totalTime * 8 / pow(10.0, 6),
            ((double)totalByte) / totalTime / pow(2.0, 20),
            ((double)totalByte) / totalTime / 1.0e3,
            ((double)totalByte) / totalTime * 8 / pow(2.0, 10));
    /*===========================================================================*/
}

int main(int argc, char **argv) {
    struct arguments args;
    int sock;

    setvbuf(stdout, NULL, _IOLBF, 0);

    if (parse_args(&args, argc, argv) < 0) {
        fprintf(stderr, "Failed to parse arguments, call with "
                        "--help for more information\n");
        return -1;
    }

    if ((sock = grnvs_open(args.interface, SOCK_RAW)) < 0) {
        fprintf(stderr, "grnvs_open() failed: %s\n", strerror(errno));
        return -1;
    }

    assignment2(sock, args.frames);

    grnvs_close(sock);

    return 0;
}
