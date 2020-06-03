/* * Kostak - A little container engine
 * Copyright (C) 2020 Didiet Noor <dnoor@ykode.com>
 *
 * This file is part of Kostak.
 *
 * Kostak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Kostak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Kostak.  If not, see <http://www.gnu.org/licenses/>.
 */

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
  bool sandbox;
  bool seccomp;

  int in_pipe[2];
  int out_pipe[2];
};

typedef struct exec_param exec_param_t;

void parse_arg(int argc, char **argv, exec_param_t *param);

#endif /* PARAM_H */
