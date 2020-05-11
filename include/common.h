#ifndef COMMON_H
#define COMMON_H 1

#define _GNU_SOURCE 1
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <net/if.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/limits.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

#include "compat.h"
#include "log.h"
#include "umm_malloc.h"

#define KBYTES (1024)

#define PANIC(...) \
  do {\
    log_fatal(__VA_ARGS__);\
    exit(EXIT_FAILURE);\
  } while (0)

#endif /* COMMON_H */
