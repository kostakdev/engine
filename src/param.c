#include "common.h"
#include "env.h"
#include "param.h"

static char *const default_rootfs = "rootfs";
static char *const default_hostname = "kostak";

static void print_usage() {
  fprintf(stderr, 
    "Usage:\tkostak [-h] [-u <hostname>] [-d -t -v] [-r rootfs] <program_name>\n"
    "\t-h\t\tshow help\n"
    "\t-u <hostname>\tcustom hostname\n"
    "\t-r rootfs"
    "\n"
    "Verbosity:\n"
    "\t-d turn on debug mode\n"
    "\t-t turn on trace mode\n"
    "\t-v turn on verbose mode\n"
  );
}

static struct option long_options[] = {
  {"help",  no_argument,        0, 'h'},
  {"uts",   required_argument,  0, 'u'},
  {"trace", no_argument,        0, 't'},
  {"debug", no_argument,        0, 'd'},
  {"verbose", no_argument,      0, 'v'},
  {"rootfs", required_argument, 0, 'r'},
  {"env", required_argument,    0, 'e'},
  {0,       0,                  0,  0 },
};

static inline void set_param_loglevel(exec_param_t *param, int new_log_level) {
  if (param->log_level > new_log_level) {
    param->log_level = new_log_level;
  }
}

void parse_arg(int argc, char **argv, exec_param_t *param)
{
  if (argc < 2) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  int c;
  int option_index = 0;
  static const char *opt_short_string = "hu:vdtr:e:";

  param->log_level = LOG_WARN;
  param->utsname = default_hostname;
  param->rootfs = default_rootfs;

  init_env_buf(&param->env);

  add_env(&param->env, "PS1=\\u@\\h:\\w\\$ ");
  add_env(&param->env, "TERM=screen");
  add_env(&param->env, "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
  
  while( -1 != (c = getopt_long(argc, argv, opt_short_string, long_options, &option_index))) {
    switch(c) {
      case 'h':
        print_usage();
        exit(EXIT_SUCCESS);
        break;
      case 'u':
        param->utsname = optarg;
        break;
      case 'r':
        param->rootfs = optarg;
        break;
      case 'v':
        set_param_loglevel(param, LOG_INFO);
        break;
      case 'd':
        set_param_loglevel(param, LOG_DEBUG);
        break;
      case 't':
        set_param_loglevel(param, LOG_TRACE);
        break;
      case 'e':
        if (NULL != strchr(optarg, '=')) {
          add_env(&param->env, optarg);
        } else {
          log_debug("Cannot add '%s' as environment variable", optarg);
        }
        break;
      case '?':
        print_usage();
        exit(EXIT_FAILURE);
        break;
      default:
        break;
    }
  }

  param->argv = &argv[optind];
  param->program_name = param->argv[0];
}