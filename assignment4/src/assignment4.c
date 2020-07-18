#include <netinet/ether.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <asm/byteorder.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>

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
                                                        sizeof(time)) < 0) {
                fprintf(stderr, "Failed to set recv timeout: %s\n",
                                strerror(errno));
                return -1;
        }

        if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&time,
                                                        sizeof(time)) < 0) {
                fprintf(stderr, "Failed to set send timeout: %s\n",
                                strerror(errno));
                return -1;
        }

        /*
         * This sets reuseaddr for the server socket, so we can create a new
         * listening server before the old one completelly dies (in case of a
         * crash or some tcp FIN_WAIT
         */
        if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &eins, sizeof(eins))<0){
                fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
                return -1;
        }
        return 0;
}


/**
 * This is the entry point for the asciiclient.
 *
 * \param ipaddr The IP address in ASCII representation. IPv6 has to be supported,
 *               IPv4 is optional
 * \param port The server port to connect to
 * \param nick The nickname to use in the protocol
 * \param msg The message to send
 */
void assignment4(const char *ipaddr, in_port_t port, char *nick, char *msg)
{
        struct in6_addr dstip;

        if( 0 > parse_ip(ipaddr, &dstip) ) {
                fprintf(stderr, "Wrong input format for destination address, "
                        "input should be in format: a:b:c::1:2:3\n");
                return;
        }

        (void) port; (void) nick; (void) msg;

/*====================================TODO===================================*/
/*===========================================================================*/
}

int main(int argc, char ** argv)
{
        struct arguments args;
        int fd;
        char buffer[4096] = { 0 };

        if ( parse_args(&args, argc, argv) < 0 ) {
                fprintf(stderr, "Failed to parse arguments, call with "
                        "--help for more information\n");
                return -1;
        }

        if(args.file) {
                args.msg = buffer;
                if((fd = open(args.file, O_RDONLY)) < 0) {
                        fprintf(stderr, "Could not open file: %s - %s\n",
                                args.file, strerror(errno));
                        return -1;
                }
                if(read(fd, buffer, sizeof(buffer)-1) < 0) {
                        fprintf(stderr, "Could not read file: %s\n",
                                strerror(errno));
                        close(fd);
                        return -1;
                }
                close(fd);
        }

        setbuf(stdout, NULL);

        assignment4(args.dst, args.port, args.nick, args.msg);

        return 0;
}
