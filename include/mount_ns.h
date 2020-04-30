#ifndef MOUNT_H
#define MOUNT_H 1

#include "param.h"

int change_root_and_mount(exec_param_t *param);
int host_mount(const char *hostpath, const char *mountpath);

#endif /* MOUNT_H */
