#ifndef NET_ETH 
#define NET_ETH 1
#include "common.h"


int create_veth(const int sock_fd, const char *ifname, const char *peername);

int prepare_netns();

#endif /* NET_ETH */