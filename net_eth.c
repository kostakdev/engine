#include "common.h"
#include "net_eth.h"
#include "netlink_util.h"

struct nl_ireq {
  struct nlmsghdr n;
  struct ifinfomsg i;
};

struct nl_areq {
  struct nlmsghdr n;
  struct ifaddrmsg a;
};

static const unsigned int UNUSED_IFI_CHANGE = 0xFFFFFFFFu;

int create_veth(const int sock_fd, const char *ifname, 
                const char *peername, const pid_t child_pid)
{
  const __u16 flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_ireq *req = (struct nl_ireq *) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWLINK;
  req->i.ifi_family = PF_NETLINK;
  req->i.ifi_change = UNUSED_IFI_CHANGE;


  struct nlmsghdr *n = &req->n;
  const int maxlen = PAYLOAD_MAX;
  addattr_string(n, maxlen, IFLA_IFNAME, ifname);
  struct rtattr *linkinfo = addattr_nest(n, maxlen, IFLA_LINKINFO);
  addattr_string(n, maxlen, IFLA_INFO_KIND, "veth");
  struct rtattr *infodata = addattr_nest(n, maxlen, IFLA_INFO_DATA);
  struct rtattr *vethinfo = addattr_nest(n, maxlen, VETH_INFO_PEER);

  struct ifinfomsg *peerinfo = reserve_space(n, maxlen, sizeof(struct ifinfomsg));
  if (NULL == peerinfo) {
    log_error("Error reserving space: %m");
    return -1;
  }

  peerinfo->ifi_family = IFA_UNSPEC;
  peerinfo->ifi_change = UNUSED_IFI_CHANGE;

  addattr_string(n, maxlen, IFLA_IFNAME, peername);
  addattr_uint32(n, maxlen, IFLA_NET_NS_PID, child_pid);
  addattr_nest_end(n, vethinfo); 
  addattr_nest_end(n, infodata);
  addattr_nest_end(n, linkinfo);

  const int status = send_nlmsg(sock_fd, (struct nlmsghdr *)buffer, true);

  if (status != -1) {
    log_debug("Successfully created veth pair %s [%ld] <--> %s [%ld]", 
      ifname, (long)getpid(), peername, (long) child_pid);
  }

  return status;
}

int if_up(const int sock_fd, const char *ifname)
{
  const int ifindex = if_nametoindex(ifname);

  if (ifindex == 0) {
    errno = ENODEV;
    return -1;
  }

  __u16 flags = NLM_F_REQUEST | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_ireq *req = (struct nl_ireq *) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWLINK;
  
  req->i.ifi_index = ifindex;
  req->i.ifi_family = AF_INET;
  req->i.ifi_flags = IFF_UP;
  req->i.ifi_change = UNUSED_IFI_CHANGE;

  addattr_string(&req->n, PAYLOAD_MAX, IFLA_IFNAME, ifname);

  const int status = send_nlmsg(sock_fd, (struct nlmsghdr *)buffer, true);

  if (status != -1) {
    log_debug("Successfully bring %s up", ifname);
  }

  return status;
}


static int parse_addrv4(const char *addr, uint32_t *ret, uint32_t *bitlen) {
  int i;
  *ret = 0;
  uint8_t *buf = (uint8_t *) ret;
  *bitlen = 24;
  char *slash = strrchr(addr, '/');

  if (NULL != slash) {
    *slash = '\0';
  }

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

    if ('\0' == *dot) 
    {
      break;
    }

    if (3 == i || '.' != *dot) {
      return -1;
    }

    addr = dot + 1;

  }

  if (NULL != slash) {
    *slash = '/';
    *bitlen = strtoul(slash + 1, NULL, 0);
  }

  return 0;
}

static int assign_address(const int sock_fd, const char *ifname, const char *addr) {
  const int ifindex = if_nametoindex(ifname);

  if (ifindex == 0) {
    errno = ENODEV;
    return -1;
  }

  uint32_t addr_i, bitlen;

  if (-1 == parse_addrv4(addr, &addr_i, &bitlen)) {
    log_error("Error parsing address");
    errno = EINVAL;
    return -1;
  }

  const __u16 flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_areq *req = (struct nl_areq*) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWADDR;

  req->a.ifa_family = AF_INET;
  req->a.ifa_prefixlen = bitlen;
  req->a.ifa_index = ifindex;

  addattr_uint32(&req->n, PAYLOAD_MAX, IFA_LOCAL, addr_i);

  return send_nlmsg(sock_fd, (struct nlmsghdr *) buffer, true);
}

static int get_netns_fd(const pid_t pid) {
  char path[PATH_MAX];
  snprintf(path, PATH_MAX, "/proc/%ld/ns/net", (long) pid);

  int fd = open(path, O_RDONLY);
  return fd;
}

int prepare_netns(const pid_t child_pid) {
  int status = 0;
  const int sock_fd = create_netlink_route_socket();
  const int cur_netns = get_netns_fd(getpid());
  const int child_netns = get_netns_fd(child_pid);

  status = create_veth(sock_fd, "kostak0", "kostak1", child_pid);

  if (-1 == assign_address(sock_fd, "kostak0", "10.0.22.1")) {
    close(sock_fd);
    return -1;
  }

  if_up(sock_fd, "kostak0");

  if (-1 == setns(child_netns, CLONE_NEWNET)) {
    log_error("Failed joining net ns for pid %ld: %m", (long) child_pid);
    close(sock_fd);
    return -1;
  }

  const int sock_fd_ns = create_netlink_route_socket();
  
  if (-1 == assign_address(sock_fd_ns, "kostak1", "10.0.22.2")) {
    close(sock_fd);
    return -1;
  }

  if_up(sock_fd_ns, "kostak1");

  close(sock_fd_ns);

  if (-1 == setns(cur_netns, CLONE_NEWNET)) {
    log_error("Failed joining net ns for pid %ld: %m", (long) getpid());
    close(sock_fd);
    return -1;
  }
  
  close(sock_fd);

  return status;
}