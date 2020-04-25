#ifndef NET_ETH 
#define NET_ETH 1
#include "common.h"


int create_veth(const int sock_fd, const char *ifname, 
                const char *peername, const pid_t child_pid);

int prepare_netns(const pid_t child_pid);

#endif /* NET_ETH */