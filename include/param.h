#ifndef PARAM_H
#define PARAM_H 1

#include "common.h"
#include "env.h"

struct port_map_entry {
  uint16_t source_port;
  uint16_t dest_port;
};

typedef struct port_map_entry port_map_entry_t;

struct exec_param {
  char **argv;
  char *program_name;
  char *utsname;
  int log_level;
  char *rootfs;

  char rootfs_mount_point[PATH_MAX];
  env_var_t env;
};
typedef struct exec_param exec_param_t;

void parse_arg(int argc, char **argv, exec_param_t *param);

#endif /* PARAM_H */