#ifndef NET_ETH 
#define NET_ETH 1
#include "common.h"

struct exec_param;
int prepare_netns(const pid_t child_pid, const struct exec_param *param);

#endif /* NET_ETH */
