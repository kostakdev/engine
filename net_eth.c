#include "net_eth.h"
#include "netlink_util.h"

struct nl_ireq {
  struct nlmsghdr n;
  struct ifinfomsg i;
};

int create_veth(const int sock_fd, const char *ifname, const char *peername)
{
  const __u16 flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_ireq *req = (struct nl_ireq *) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWLINK;
  req->i.ifi_family = PF_NETLINK;

  struct nlmsghdr *n = &req->n;
  const int maxlen = PAYLOAD_MAX;
  addattr_string(n, maxlen, IFLA_IFNAME, ifname);
  struct rtattr *linkinfo = addattr_nest(n, maxlen, IFLA_LINKINFO);
  addattr_string(n, maxlen, IFLA_INFO_KIND, "veth");
  struct rtattr *infodata = addattr_nest(n, maxlen, IFLA_INFO_DATA);
  struct rtattr *vethinfo = addattr_nest(n, maxlen, VETH_INFO_PEER);
  n->nlmsg_len += sizeof(struct ifinfomsg);
  addattr_string(n, maxlen, IFLA_IFNAME, peername);
  addattr_nest_end(n, vethinfo); 
  addattr_nest_end(n, infodata);
  addattr_nest_end(n, linkinfo);

  return send_nlmsg(sock_fd, (struct nlmsghdr *)buffer, true);
}

int prepare_netns() {
  int status = 0;
  const int sock_fd = create_socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);

  status = create_veth(sock_fd, "kostak0", "kostak1");

  close(sock_fd);

  return status;
}