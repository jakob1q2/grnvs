#include <argp.h>
#include <stdio.h>
#include <net/ethernet.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "arguments.h"


static char args_doc[] = "<nick> <destination>";

static char doc[] =
	"  <nick>         the nick that should be displayed on the server\n"
	"  <destination>  the destination IPv6 address\n"
	"\n"
	"  -m <message> or -f <file> must be given!\n";

enum fix_args {
	FIX_ARG_NICK = 0,
	FIX_ARG_DST
};

static struct argp_option options[] = {
	{
		"file",
		'f',
		"file",
		0,
		"a file to use as message, -m will be ignored",
		0
	},
	{
		"port",
		'p',
		"port",
		0,
		"the port the client should connect to (optional, default: 1337)",
		0
	},
	{
		"message",
		'm',
		"message",
		0,
		"the message to send, spaces might need to be quoted in the shell",
		0
	},
	{ 0 }
};

static error_t parse_opt(int key, char * arg, struct argp_state * state);

static struct argp argp = {
	options,
	parse_opt,
	args_doc,
	doc,
	0,
	0,
	0
};

static error_t parse_opt(int key, char * arg, struct argp_state * state)
{
	struct arguments * args = state->input;
	char * ptr;

	switch (key) {
	case ARGP_KEY_ARG:
		switch(state->arg_num) {
		case FIX_ARG_DST:
			args->dst = arg;
			break;
		case FIX_ARG_NICK:
			args->nick = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
		}
		break;
	case 'm':
		args->msg = arg;
		break;
	case 'p':
		args->port = (int)strtol(arg, &ptr, 10);
		if(*ptr)
			return EINVAL;
		break;
	case 'f':
		args->file = arg;
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int parse_args(struct arguments * args, int argc, char ** argv)
{
	memset(args, 0, sizeof(*args));
	args->port = 1337;
	if(argp_parse(&argp, argc, argv, 0, 0, args))
		return -1;
	if(!args->dst)
		return -1;
	if(!args->nick)
		return -1;
	if(!args->file && !args->msg)
		return -1;

	return 0;
}
