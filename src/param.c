#include "common.h"
#include "param.h"

static char *const default_rootfs = "rootfs";
static char *const default_hostname = "kostak";

static void print_usage() {
  fprintf(stderr, 
    "Usage:\tkostak [-h] [-u <hostname>] [-d -t -v] [-r rootfs] [-p port_mapping] [-e env_vars] [-m mount_mapping] <program_name>\n"
    "\t-h\t\t\tshow help\n"
    "\t-u <hostname>\t\tcustom hostname\n"
    "\t-r rootfs\t\tspecify the root filesystem\n"
    "\t-p <dest>:<host>\tport mapping from dest to host \n"
    "\t-m <hostdir>:<mount>\tmounting host directory to container\n"
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

static const char *default_args[] = {
  "PS1=\\u@\\h:\\w\\$ ",
  "TERM=screen",
  "PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin",
};

void parse_arg(int argc, char **argv, exec_param_t *param)
{
  if (argc < 2) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  int c;
  int option_index = 0;
  static const char *opt_short_string = "hu:vdtr:e:p:m:";

  param->log_level = LOG_WARN;
  param->utsname = default_hostname;
  param->rootfs = default_rootfs;

  init_ptr_vec(&param->env, (void**) default_args, 
    sizeof(default_args) / sizeof(const char*));
  
  init_ptr_vec(&param->port_maps, NULL ,0);
  init_ptr_vec(&param->mounts, NULL, 0);
  
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
          add_ptr(&param->env, optarg);
        } else {
          log_debug("Cannot add '%s' as environment variable", optarg);
        }
        break;
      case 'p':
        if (NULL != strchr(optarg, ':')) {
          add_ptr(&param->port_maps, optarg);
        } else {
          log_debug("%s is not a port mapping syntax", optarg);
        }
        break;
      case 'm':
        if (NULL != strchr(optarg, ':')) {
          add_ptr(&param->mounts, optarg);
        } else {
          log_debug("%s is not a volume mapping syntax", optarg);
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
