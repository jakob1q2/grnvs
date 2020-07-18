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
#define BUFSIZE 4096

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
int parse_ip(const char *ipaddr, struct in6_addr *dst_ip)
{
    return inet_pton(AF_INET6, ipaddr, dst_ip) - 1;
}

/*
 * Use this function to set a timeout on a socket.
 * Once a timeout is set, the socket operations will time out after the
 * specified time (given in seconds)
 *
 * The timeout will reset between function calls.
 */
int set_socket_options(int socket, int timeout)
{
    struct timeval time;
    int eins = 1;
    time.tv_sec = timeout;
    time.tv_usec = 0;

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time,
                   sizeof(time)) < 0)
    {
        fprintf(stderr, "Failed to set recv timeout: %s\n", strerror(errno));
        return -1;
    }

    if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&time,
                   sizeof(time)) < 0)
    {
        fprintf(stderr, "Failed to set send timeout: %s\n", strerror(errno));
        return -1;
    }

    /*
     * This sets reuseaddr for the server socket, so we can create a new
     * listening server before the old one completelly dies (in case of a
     * crash or some tcp FIN_WAIT
     */
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &eins, sizeof(eins)) < 0)
    {
        fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * counts number of digits of decimal number
 */
u_int16_t countDigits(size_t c)
{
    u_int16_t digits = 1;
    while ((c /= 10) != 0)
    {
        ++digits;
    }
    return digits;
}

/**
 * writes pure String src into dest as Netstring and returns length of Netstring
 * dest cannot be src!
 */

size_t writeNet(char *dest, size_t bufSize, char *src, int fd)
{
    memset(dest, 0, bufSize);
    size_t len = bufSize;
    if ((len = strlen(src)) < bufSize - countDigits(len))
    {
        sprintf(dest, "%ld:", len);
        strcat(dest, src);
        strcat(dest, ",");
    }
    else
    {
        sprintf(dest, "0:");
    }
    return len + countDigits(len) + 1;
}

/**
 * extracts pure String from Netstring src into dest and returns length of pure
 * String
 */
size_t readNet(char *dest, size_t bufSize, char *src, int fd)
{
    //check netstring format
    if (strchr(src, ':') == NULL || strchr(src, ',') == NULL || strchr(src, ':') > strchr(src, ','))
    {
        checkMessage(fd, "String in netstring format", src);
    }

    //refuse leading 0 (also message cannot be 0 long)
    if (src[0] == '0')
    {
        checkMessage(fd, "no leading 0", src);
    }

    //convert
    int pos = strchr(src, ':') - src;
    char num[pos + 1];
    memset(num, 0, pos + 1);
    strncpy(num, src, pos);
    size_t strLength = atol(num);
    if (strLength > bufSize)
    {
        return -1;
    }
    strncpy(dest, src + pos + 1, strLength);
    dest[strLength] = 0;
    return strLength;
}

/**
 * writes string message m to socket descriptor using buffer for netstring conversion
 */
int sendMessage(int fd, char *m, char *buf)
{
    return write(fd, buf, writeNet(buf, BUFSIZE, m, fd) + 1); //remember 0 Byte at end
}

/**
 * receives netstring message m from socket descriptor and stores message in respDst
 */

int recvMessage(int fd, char *respDst)
{
    memset(respDst, 0, BUFSIZE);
    read(fd, respDst, BUFSIZE);
    return readNet(respDst, BUFSIZE, respDst, fd);
}

int checkMessage(int fd, char *expected, char *actual)
{
    if (actual[0] == 'E') //server sent error
    {
        printf("Error: %s", actual + 2);
    }
    else if (strcmp(expected, actual) != 0) //client sends error
    {
        char errmsg[BUFSIZE] = {0};
        strcpy(errmsg, "E ");
        strcat(errmsg, "expected: \"");
        strcat(errmsg, expected);
        strcat(errmsg, "\" but received: \"");
        strcat(errmsg, actual);
        strcat(errmsg, "\"");
        sendMessage(fd, errmsg, actual);
        printf("Error: %s", errmsg + 2);
        exit(-1);
    }
    return 0;
}

/**
 * This is the entry point for the asciiclient.
 *
 * \param ipaddr The IP address in ASCII representation. IPv6 has to be
 * supported, IPv4 is optional \param port The server port to connect to \param
 * nick The nickname to use in the protocol \param msg The message to send
 */
void assignment4(const char *ipaddr, in_port_t port, char *nick, char *msg)
{
    struct in6_addr dstip;

    if (0 > parse_ip(ipaddr, &dstip))
    {
        fprintf(stderr, "Wrong input format for destination address, "
                        "input should be in format: a:b:c::1:2:3\n");
        return;
    }

    /*====================================TODO===================================*/
    struct sockaddr_in6 control = {};
    int sdC;

    struct sockaddr_in6 listen_sock = {};
    int sdL;

    char buf[BUFSIZE] = {0};
    char token[BUFSIZE] = {0};
    char text[BUFSIZE] = {0};

    //set up control socket
    control.sin6_family = AF_INET6;
    control.sin6_port = htons(port);
    control.sin6_addr = dstip;

    if (0 > (sdC = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("error in socket() on control socket");
        exit(1);
    }

    int ret = connect(sdC, (struct sockaddr *)&control, sizeof(control));

    if (0 > ret)
    {
        close(sdC);
        perror("error in connect() on control socket");
        exit(1);
    }

    //start protocol on control socket
    sendMessage(sdC, "C GRNVS V:1.0", buf);
    recvMessage(sdC, buf);
    checkMessage(sdC, "S GRNVS V:1.0", buf);

    strcpy(text, "C ");
    strcat(text, nick);
    sendMessage(sdC, text, buf);
    recvMessage(sdC, buf);
    /**if (buf[0] != 'S' || buf[1] != ' ')
        checkMessage(sdC, "S <token>", buf);
        */
    strcpy(token, buf + 2);

    //set up listen socket
    listen_sock.sin6_family = AF_INET6;
    listen_sock.sin6_port = htons(33172); // auto assign port;
    listen_sock.sin6_addr = in6addr_any;

    if (0 > (sdL = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("error in socket() on data socket");
        exit(1);
    }

    ret = bind(sdL, (struct sockaddr *)&listen_sock, sizeof(listen_sock));

    if (0 > ret)
    {
        close(sdC);
        close(sdL);
        perror("error in bind() on data socket");
        exit(1);
    }

    set_socket_options(sdL, 2);

    //tell server where to connect
    strcpy(text, "C ");
    sprintf(buf, "%d", ntohs(listen_sock.sin6_port));
    strcat(text, buf);
    sendMessage(sdC, text, buf);

    ret = listen(sdL, 1);
    if (ret < 0)
    {
        close(sdC);
        close(sdL);
        perror("error in listen() on data socket");
        exit(1);
    }

    struct sockaddr_in6 data = {};
    int sdD;
    socklen_t data_len = sizeof(data);

    sdD = accept(sdL,
                 (struct sockaddr *)&data,
                 &data_len);
    if (sdD < 0)
    {
        close(sdC);
        close(sdL);
        perror("error in accept() on data socket");
        exit(1);
    }

    set_socket_options(sdD, 2);
    recvMessage(sdD, buf);
    checkMessage(sdD, "T GRNVS V:1.0", buf);

    strcpy(text, "D ");
    strcat(text, nick);
    sendMessage(sdD, text, buf);

    recvMessage(sdD, buf);
    strcpy(text, "T ");
    strcat(text, token);
    checkMessage(sdD, text, buf);

    strcpy(text, "D ");
    strcat(text, msg);
    sendMessage(sdD, text, buf);

    recvMessage(sdD, buf);
    /**
    if (buf[0] != 'T' || buf[1] != ' ')
        checkMessage(sdD, "T <dtoken>", buf);
    */
    strcpy(token, buf + 2);

    close(sdL);
    close(sdD);

    recvMessage(sdC, buf);
    strcpy(text, "S ");
    sprintf(text + 2, "%ld", strlen(msg));
    checkMessage(sdC, text, buf);

    strcpy(text, "C ");
    strcat(text, token);
    sendMessage(sdC, text, buf);

    recvMessage(sdC, buf);
    strcpy(text, "S ACK");
    checkMessage(sdC, text, buf);

    close(sdC);
    /*===========================================================================*/
}

int main(int argc, char **argv)
{
    struct arguments args;
    int fd;
    char buffer[4096] = {0};

    if (parse_args(&args, argc, argv) < 0)
    {
        fprintf(stderr, "Failed to parse arguments, call with "
                        "--help for more information\n");
        return -1;
    }

    if (args.file)
    {
        args.msg = buffer;
        if ((fd = open(args.file, O_RDONLY)) < 0)
        {
            fprintf(stderr, "Could not open file: %s - %s\n", args.file,
                    strerror(errno));
            return -1;
        }
        if (read(fd, buffer, sizeof(buffer) - 1) < 0)
        {
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
