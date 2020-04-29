#include "common.h"
#include "netlink_util.h"

static inline struct rtattr *nlmsg_tail(struct nlmsghdr *n)
{
  return (struct rtattr *)((uint8_t *)n + NLMSG_ALIGN(n->nlmsg_len));
}

void *reserve_space(struct nlmsghdr *n, int maxlen, size_t sz) 
{
  const ssize_t newlen = NLMSG_ALIGN(n->nlmsg_len) + NLMSG_ALIGN(sz);

  if (newlen > maxlen) {
    errno = ENOSPC;
    return NULL;
  }

  n->nlmsg_len = newlen;
  void *buf = nlmsg_tail(n);
  memset(buf, 0, sz);
  return buf;
}

int addattr_l(struct nlmsghdr *n,
              int maxlen, __u16 type,
              const void *data, __u16 alen)
{
  const __u16 len = RTA_LENGTH(alen);
  const ssize_t newlen = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);

  if (newlen > maxlen)
  {
    errno = ENOSPC;
    return -1;
  }

  struct rtattr *attr = nlmsg_tail(n);
  attr->rta_len = len;
  attr->rta_type = type;

  void *rta_data = RTA_DATA(attr);

  memmove(rta_data, data, alen);

  n->nlmsg_len = newlen;

  return 0;
}

struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, __u16 type)
{
  struct rtattr *attr = nlmsg_tail(n);

  if (-1 == addattr_l(n, maxlen, type, NULL, 0))
  {
    log_error("Failed to add attributes\n");
    return NULL;
  }
  return attr;
}

void addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest)
{
  const void *tail = nlmsg_tail(n);
  nest->rta_len = (uint8_t *)tail - (uint8_t *)nest;
}

static ssize_t read_response(int fd, struct msghdr *msg, char **response)
{
  struct iovec *iov = msg->msg_iov;
  iov->iov_base = *response;
  iov->iov_len = PAYLOAD_MAX;

  const ssize_t resp_len = recvmsg(fd, msg, 0);

  if (0 == resp_len)
  {
    log_error("Premature EOF of NETLINK\n");
    errno = ENODATA;
    return -1;
  }

  if (0 > resp_len)
  {
    log_error("Netlink receive error\n");
    errno = ECONNRESET;
    return -1;
  }

  return resp_len;
}

int check_response(int sock_fd)
{
  struct iovec iov;

  struct msghdr msg = {
      .msg_name = NULL,
      .msg_namelen = 0,
      .msg_iov = &iov,
      .msg_iovlen = 1,
  };

  char *resp = alloca(PAYLOAD_MAX);

  ssize_t resp_len = read_response(sock_fd, &msg, &resp);

  struct nlmsghdr *hdr = (struct nlmsghdr *)resp;

  int nlmsglen = hdr->nlmsg_len;
  int datalen = nlmsglen - (sizeof(*hdr));

  if (datalen < 0 || nlmsglen > resp_len)
  {
    if (msg.msg_flags & MSG_TRUNC)
    {
      log_error("Received truncated message.");
      errno = EBADMSG;
      return -1;
    }

    log_error("MALFORMED MESSAGE");
    errno = EBADMSG;
    return -1;
  }

  if (hdr->nlmsg_type == NLMSG_ERROR)
  {
    struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(hdr);

    if (datalen < (int) sizeof(struct nlmsgerr))
    {
      log_error("Error truncated");
      return -1;
    }

    if (err->error)
    {
      errno = -err->error;
      log_error("RTNETLink Error: %m");
      return -1;
    }
  }
  return 0;
}

int send_nlmsg(int sock_fd, struct nlmsghdr *n, const bool ack)
{
  struct iovec iov = {
      .iov_base = n,
      .iov_len = n->nlmsg_len};

  struct msghdr msg = {
      .msg_name = NULL,
      .msg_namelen = 0,
      .msg_iov = &iov,
      .msg_iovlen = 1};

  ++n->nlmsg_seq;

  const ssize_t status = sendmsg(sock_fd, &msg, 0);
  if (status < 0) {
    log_error("cannot talk to rtnetlink: %m\n");
    return -1;
  }

  if (ack)
  {
    return check_response(sock_fd);
  }

  return status;
}

int parse_addrv4(const char *addr, uint32_t *ret, uint32_t *bitlen) {
  int i;
  *ret = 0;
  uint8_t *buf = (uint8_t *) ret;
  if (NULL != bitlen) {
    *bitlen = 24;
  }
  char *slash = strrchr(addr, '/');

  for (i = 0; i < 4; ++i) {
    unsigned long n;
    char *dot;

    n = strtoul(addr, &dot, 0);

    if (n > 255) {
      return -1;
    }

    if (dot == addr) {
      return -1;
    }

    buf[i] = n;

    if ('\0' == *dot || '/' == *dot) {
      break;
    }

    if (3 == i || '.' != *dot) {
      return -1;
    }

    addr = dot + 1;

  }

  if (NULL != slash) {
    if (NULL != bitlen) {
      *bitlen = strtoul(slash + 1, NULL, 0);
    }
  }

  return 0;
}
