#ifndef NETLINK_UTIL_H
#define NETLINK_UTIL_H 1

#include "common.h"

static size_t PAYLOAD_MAX = 1024;

int addattr_l(struct nlmsghdr *n, int maxlen, __u16 type, const void *data, __u16 alen);
struct rtattr* addattr_nest(struct nlmsghdr *n, int maxlen, __u16 type);
void addattr_nest_end(struct nlmsghdr *n, struct rtattr *);

static inline int addattr_uint32(struct nlmsghdr *n, int maxlen, __u16 type, 
                                 const uint32_t v) 
{
  return addattr_l(n, maxlen, type, &v, sizeof(uint32_t));
}

static inline int addattr_uint16(struct nlmsghdr *n, int maxlen, __u16 type,
                                 const uint16_t v)
{
  return addattr_l(n, maxlen, type, &v, sizeof(uint16_t));
}

static inline int addattr_string(struct nlmsghdr *n, int maxlen, __u16 type, const char *s)
{
  return addattr_l(n, maxlen, type, s, strlen(s) + 1);
}

static inline int create_socket(int domain, int type, int protocol) 
{
  int sock_fd = socket(domain, type, protocol);
  if ( 0 > sock_fd) {
    log_error("Cannot open socket: %m");
  }

  return sock_fd;
}

int send_nlmsg(const int sock_fd, struct nlmsghdr *n, const bool ack);




#endif /* NETLINK_UTIL_H */