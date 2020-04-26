#include "common.h"
#include "net_eth.h"
#include "netlink_util.h"

struct nl_req {
  struct nlmsghdr n;
  union {
    struct ifinfomsg i;
    struct ifaddrmsg a;
    struct rtmsg r;
    struct nfgenmsg f;
  };
};

static const unsigned int UNUSED_IFI_CHANGE = 0xFFFFFFFFu;

int create_veth(const int sock_fd, 
                const char *ifname, 
                const char *peername, 
                const pid_t child_pid)
{
  const __u16 flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_req *req = (struct nl_req *) buffer;
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

int if_up_down(const int sock_fd, const char *ifname, bool up)
{
  const int ifindex = if_nametoindex(ifname);

  if (ifindex == 0) {
    errno = ENODEV;
    return -1;
  }

  const __u16 flags = NLM_F_REQUEST | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_req *req = (struct nl_req *) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWLINK;
  
  req->i.ifi_index = ifindex;
  req->i.ifi_family = AF_INET;
  req->i.ifi_flags = up ? IFF_UP : ~IFF_UP;
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
  if (NULL != bitlen) {
    *bitlen = 24;
  }
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

    if ('\0' == *dot) {
      break;
    }

    if (3 == i || '.' != *dot) {
      return -1;
    }

    addr = dot + 1;

  }

  if (NULL != slash) {
    *slash = '/';
    if (NULL != bitlen) {
      *bitlen = strtoul(slash + 1, NULL, 0);
    }
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

  struct nl_req *req = (struct nl_req*) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWADDR;

  req->a.ifa_family = AF_INET;
  req->a.ifa_prefixlen = bitlen;
  req->a.ifa_index = ifindex;

  addattr_uint32(&req->n, PAYLOAD_MAX, IFA_LOCAL, addr_i);

  return send_nlmsg(sock_fd, (struct nlmsghdr *) buffer, true);
}

const size_t TABLE_CLEAR_SIZE = sizeof(struct nlmsghdr) + sizeof(struct nfgenmsg);

static int start_nf_batch(void *buf, void **nextbuf) {
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST;
  req->n.nlmsg_type = NFNL_MSG_BATCH_BEGIN;
  req->n.nlmsg_seq = time(NULL);

  req->f.res_id = NFNL_SUBSYS_NFTABLES;

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;
}

static int end_nf_batch(void *buf, void **nextbuf) {
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST;
  req->n.nlmsg_type = NFNL_MSG_BATCH_END;
  req->n.nlmsg_seq = time(NULL);

  req->f.res_id = NFNL_SUBSYS_NFTABLES;

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;
}

static int flush_rules(void *buf, void **nextbuf) {
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_DELTABLE;
  req->n.nlmsg_seq = time(NULL);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0; 
}

static int create_table(const char *table_name, void *buf, void **nextbuf)
{
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE;
  req->n.nlmsg_seq = time(NULL);

  req->f.nfgen_family = AF_INET;

  addattr_string(&req->n, PAYLOAD_MAX, NFTA_TABLE_NAME, table_name);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;  
}

static int create_chain(const char *table_name, const char *chain_name, 
                        void *buf, void **nextbuf)
{
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWCHAIN;
  req->n.nlmsg_seq = time(NULL);

  req->f.nfgen_family = AF_INET;

  struct nlmsghdr *n = &req->n;

  addattr_string(n, PAYLOAD_MAX, NFTA_CHAIN_TABLE, table_name);
  addattr_string(n, PAYLOAD_MAX, NFTA_CHAIN_NAME, chain_name);
  addattr_string(n, PAYLOAD_MAX, NFTA_CHAIN_TYPE, "nat");
  addattr_uint32(n, PAYLOAD_MAX, NFTA_CHAIN_POLICY, htonl(NF_ACCEPT));

  struct rtattr *hookdata = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_CHAIN_HOOK);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_HOOK_HOOKNUM, htonl(NF_INET_POST_ROUTING));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_HOOK_PRIORITY, htonl(100));
  addattr_nest_end(n, hookdata);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;  
}

static int create_masq_rule(const char *table_name, const char *chain_name, 
                        void *buf, void **nextbuf)
{
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWRULE;
  req->n.nlmsg_seq = time(NULL);

  req->f.nfgen_family = AF_INET;

  struct nlmsghdr *n = &req->n;

  addattr_string(n, PAYLOAD_MAX, NFTA_RULE_TABLE, table_name);
  addattr_string(n, PAYLOAD_MAX, NFTA_RULE_CHAIN, chain_name);

  struct rtattr *rule = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_RULE_EXPRESSIONS);
  struct rtattr *elem_masq = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "masq");
  addattr_nest_end(n, elem_masq);
  addattr_nest_end(n, rule);
  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;  
}



static int add_nat(int sock_fd) {
  uint8_t buf[PAYLOAD_MAX];
  void *next = buf;

  start_nf_batch(next, &next);
  flush_rules(next, &next);
  create_table("kostak", next, &next);
  create_chain("kostak", "postrouting", next, &next);
  create_masq_rule("kostak", "postrouting", next, &next);
  
  end_nf_batch(next, &next);

  const size_t payload_size = (uint8_t *)next - buf;

  struct iovec iov = {
      .iov_base = buf,
      .iov_len = payload_size,
  };
  struct msghdr msgheader = {
    .msg_name = NULL,
    .msg_namelen = 0,
    .msg_iov = &iov,
    .msg_iovlen = 1,
  };

  const ssize_t status = sendmsg(sock_fd, &msgheader, 0);
  if (status < 0) {
    log_error("Cannot talk to rtlink: %m");
    return status;
  }

  return check_response(sock_fd);
}

static int create_default_route(const int sock_fd, const char *oifname, const char *next_hop)
{
  const int ifindex = if_nametoindex(oifname);

  if (ifindex == 0) {
    errno = ENODEV;
    return -1;
  }

  uint32_t addr_i;

  if (-1 == parse_addrv4(next_hop, &addr_i, NULL)) {
    log_error("Error parsing address");
    errno = EINVAL;
    return -1;
  }

  const __u16 flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_req *req = (struct nl_req*) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWROUTE;

  req->r.rtm_family = AF_INET;

  req->r.rtm_table = RT_TABLE_MAIN;
  req->r.rtm_protocol = RTPROT_STATIC;
  req->r.rtm_scope = RT_SCOPE_UNIVERSE;
  req->r.rtm_type = RTN_UNICAST;

  addattr_uint32(&req->n, PAYLOAD_MAX, RTA_GATEWAY, addr_i);
  addattr_uint32(&req->n, PAYLOAD_MAX, RTA_OIF, ifindex);

  return send_nlmsg(sock_fd, (struct nlmsghdr *) buffer, true);
} 

static int get_netns_fd(const pid_t pid) {
  char path[PATH_MAX];
  snprintf(path, PATH_MAX, "/proc/%ld/ns/net", (long) pid);

  int fd = open(path, O_RDONLY);
  return fd;
}

static int enable_ip_forwarding() {
  static const char *filename = "/proc/sys/net/ipv4/ip_forward";
  static char enabled = '1';

  const int fd = open(filename, O_RDWR);

  if (fd < 0) {
    log_error("Error opening file for ip forwarding: %m");
    return -1;
  }

  if (sizeof(char) != write(fd, &enabled, sizeof(char))) {
    log_error("Error writing file for ip forwarding: %m");
    close(fd);
    return -1;
  }

  return close(fd); 
}

int prepare_netns(const pid_t child_pid) {
  int status = 0;
  const int sock_fd = create_netlink_route_socket();
  const int cur_netns_fd = get_netns_fd(getpid());
  const int child_netns_fd = get_netns_fd(child_pid);

  status = create_veth(sock_fd, "kostak0", "kostak1", child_pid);

  if (-1 == assign_address(sock_fd, "kostak0", "10.0.22.1")) {
    close(sock_fd);
    return -1;
  }

  if_up_down(sock_fd, "kostak0", true);

  if (-1 == setns(child_netns_fd, CLONE_NEWNET)) {
    log_error("Failed joining net ns for pid %ld: %m", (long) child_pid);
    close(sock_fd);
    return -1;
  }

  const int sock_fd_ns = create_netlink_route_socket();
  
  if (-1 == assign_address(sock_fd_ns, "kostak1", "10.0.22.2")) {
    close(sock_fd);
    return -1;
  }

  if_up_down(sock_fd_ns, "kostak1", true);
  
  
  if (-1 == create_default_route(sock_fd_ns, "kostak1", "10.0.22.1")) {
    log_error("Error creating route: %m");
    close(sock_fd_ns);
    return -1;
  }

  close(sock_fd_ns);

  if (-1 == setns(cur_netns_fd, CLONE_NEWNET)) {
    log_error("Failed joining net ns for pid %ld: %m", (long) getpid());
    close(sock_fd);
    return -1;
  }
  close(child_netns_fd);
  close(cur_netns_fd);

  if (-1 == enable_ip_forwarding()) {
    close(sock_fd);
    return -1;
  }
  
  close(sock_fd);

  const int netfilter_sock = create_socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_NETFILTER);

  add_nat(netfilter_sock);

  close(netfilter_sock);

  return status;
}