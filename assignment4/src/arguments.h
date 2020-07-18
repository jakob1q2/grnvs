#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

struct arguments {
	char *nick;
	char *dst;
	int port;
	char *msg;
	char *file;
};

int parse_args(struct arguments * args, int argc, char ** argv);

#endif /*_ARGUMENTS_H_*/
