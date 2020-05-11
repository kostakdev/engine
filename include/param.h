#ifndef PARAM_H
#define PARAM_H 1

#include "common.h"
#include "util.h"

struct exec_param {
  char **argv;
  char *program_name;
  char *utsname;
  int log_level;
  char *rootfs;
  char rootfs_mount_point[PATH_MAX];
  ptr_vec_t env;
  ptr_vec_t mounts;
  ptr_vec_t port_maps;

  int in_pipe[2];
  int out_pipe[2];
};

typedef struct exec_param exec_param_t;

void parse_arg(int argc, char **argv, exec_param_t *param);

#endif /* PARAM_H */
