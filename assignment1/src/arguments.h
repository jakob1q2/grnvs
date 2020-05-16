#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

struct arguments {
	long start;
};

int parse_args(struct arguments * args, int argc, char ** argv);

#endif /*_ARGUMENTS_H_*/
