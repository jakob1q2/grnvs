#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <errno.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "arguments.h"

#define SIZESIZE 4

#define min(x, y) ((x) < (y) ? (x) : (y))

/*
 * This function parses the ip from an address string into a machine readable
 * format.
 *
 * @param dst_ip A pointer to a struct in6_addr where the ip will be written to
 * @param ipaddr The ip address as null-terminated string
 *
 * @return  0 on success
 *         -1 on error
 */
int parse_ip(const char *ipaddr, struct in6_addr *dst_ip) {
    return inet_pton(AF_INET6, ipaddr, dst_ip) - 1;
}

/*
 * Use this function to set a timeout on a socket.
 * Once a timeout is set, the socket operations will time out after the
 * specified time (given in seconds)
 *
 * The timeout will reset between function calls.
 */
int set_socket_options(int socket, int timeout) {
    struct timeval time;
    int eins = 1;
    time.tv_sec = timeout;
    time.tv_usec = 0;

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time,
                   sizeof(time)) < 0) {
        fprintf(stderr, "Failed to set recv timeout: %s\n", strerror(errno));
        return -1;
    }

    if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&time,
                   sizeof(time)) < 0) {
        fprintf(stderr, "Failed to set send timeout: %s\n", strerror(errno));
        return -1;
    }

    /*
     * This sets reuseaddr for the server socket, so we can create a new
     * listening server before the old one completelly dies (in case of a
     * crash or some tcp FIN_WAIT
     */
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &eins, sizeof(eins)) < 0) {
        fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

u_int16_t countDigits(size_t c) {
    u_int16_t digits = 1;
    while ((c /= 10) != 0) {
        ++digits;
    }
    return digits;
}

/**
 * writes pure String src into dest as Netstring and returns length of Netstring
 */

size_t writeNet(char *dest, size_t bufSize, char *src) {
    memset(dest, 0, bufSize);
    size_t len;
    if ((len = strlen(src)) < bufSize - countDigits(len)) {
        sprintf(dest, "%ld:", len);
        strcat(dest, src);
    } else {
        sprintf(dest, "0:");
    }
    return len + countDigits(len) + 1;
}

/**
 * extracts pure String from Netstring src into dest and returns length of pure
 * String
 */
size_t readNet(char *dest, size_t bufSize, char *src) {
    int pos = strchr(src, ':') - src;
    char num[pos + 1];
    memset(num, 0, pos + 1);
    strncpy(num, src, pos);
    size_t strLength = atol(num);
    if (strLength > bufSize) {
        return -1;
    }
    strncpy(dest, src + pos + 1, strLength);
    dest[strLength] =
        0; // 0 terminate string in case buf was filled with random stuff
    return strLength;
}

/**
 * This is the entry point for the asciiclient.
 *
 * \param ipaddr The IP address in ASCII representation. IPv6 has to be
 * supported, IPv4 is optional \param port The server port to connect to \param
 * nick The nickname to use in the protocol \param msg The message to send
 */
void assignment4(const char *ipaddr, in_port_t port, char *nick, char *msg) {
    struct in6_addr dstip;

    if (0 > parse_ip(ipaddr, &dstip)) {
        fprintf(stderr, "Wrong input format for destination address, "
                        "input should be in format: a:b:c::1:2:3\n");
        return;
    }

    (void)port;
    (void)nick;
    (void)msg;

    /*====================================TODO===================================*/
    struct sockaddr_in6 control;
    int sdC;
    struct sockaddr_in6 data;
    int sdD;

    size_t bufSize = 4096;
    char buf[bufSize];

    control.sin6_family = AF_INET6;
    control.sin6_port = htons(port);
    control.sin6_addr = dstip;

    if (0 > (sdC = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP))) {
        perror("error in socket()");
        exit(1);
    }
    if (0 > connect(sdC, (struct sockaddr *)&control, sizeof(control))) {
        perror("error in connect()");
        exit(1);
    }

    data.sin6_family = AF_INET6;
    data.sin6_port = htons(0); // auto assign port;
    data.sin6_addr = dstip;

    write(sdC, buf, writeNet(buf, bufSize, "C GRNVS V:1.0"));
    read(sdC, buf, 256);
    printf("%s", buf);

    /*===========================================================================*/
}

int main(int argc, char **argv) {
    struct arguments args;
    int fd;
    char buffer[4096] = {0};

    if (parse_args(&args, argc, argv) < 0) {
        fprintf(stderr, "Failed to parse arguments, call with "
                        "--help for more information\n");
        return -1;
    }

    if (args.file) {
        args.msg = buffer;
        if ((fd = open(args.file, O_RDONLY)) < 0) {
            fprintf(stderr, "Could not open file: %s - %s\n", args.file,
                    strerror(errno));
            return -1;
        }
        if (read(fd, buffer, sizeof(buffer) - 1) < 0) {
            fprintf(stderr, "Could not read file: %s\n", strerror(errno));
            close(fd);
            return -1;
        }
        close(fd);
    }

    setbuf(stdout, NULL);

    assignment4(args.dst, args.port, args.nick, args.msg);

    return 0;
}
