#include "common.h"
#include "netlink_util.h"
#include "net_forward.h"

const size_t TABLE_CLEAR_SIZE = sizeof(struct nlmsghdr) + sizeof(struct nfgenmsg);

int start_nf_batch(void *buf, void **nextbuf) {
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

int end_nf_batch(void *buf, void **nextbuf) {
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

int flush_rules(void *buf, void **nextbuf) {
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

int create_table(const char *table_name, void *buf, void **nextbuf)
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

int create_nat_chain(const char *table_name, const char *chain_name,
                     const nat_hook_chain_t hook_type,
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
  addattr_uint32(n, PAYLOAD_MAX, NFTA_HOOK_HOOKNUM, htonl(hook_type));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_HOOK_PRIORITY, htonl(100));
  addattr_nest_end(n, hookdata);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;    
}

int create_masq_rule(const char *table_name, const char *chain_name, 
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

int create_tcp_portforward_rule(const char *table_name, const char *chain_name,
  const __u16 dest_port, const char *dest_addr, __attribute__((unused)) const __u16 target_port, 
  void *buf, void **nextbuf)
{
  uint32_t addr_i = 0;
  if (-1 == parse_addrv4(dest_addr, &addr_i, NULL)) {
    log_error("Error parsing address");
    errno = EINVAL;
    return -1;
  }

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


  // tcp dport 8000 dnat to 10.0.22.2

  /* comparing protocol */
  /* NFT_META_L4_PROTO == IPPROTO_TCP 
   *
   * load l4proto => r1 (d)
   * cmp (s) r1, IPPROTO TCP
   */

  struct rtattr *rule = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_RULE_EXPRESSIONS);
  struct rtattr *elem_meta = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "meta");
  struct rtattr *meta_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_META_DREG, htonl(1));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_META_KEY, htonl(NFT_META_L4PROTO));
  addattr_nest_end(n, meta_expr_data);
  addattr_nest_end(n, elem_meta);

  struct rtattr *elem_cmp_proto = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "cmp");
  struct rtattr *cmp_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_CMP_SREG, htonl(1));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_CMP_OP, htonl(NFT_CMP_EQ));
  struct rtattr *cmp_eq_value = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_CMP_DATA);
  addattr_uint8(n, PAYLOAD_MAX, NFTA_DATA_VALUE, IPPROTO_TCP);
  addattr_nest_end(n, cmp_eq_value); 
  addattr_nest_end(n, cmp_expr_data);
  addattr_nest_end(n, elem_cmp_proto);


  
  /* comparing destination port */
  /* dest_port == 8000
   *
   * load dest_port => r1 (d)
   * cmp (s) r1, 8000
   */
  
  /* TCP HEADER
   0                      2                    4
   | 2 byte - source port | 2 byte - dest port |
  */

  struct rtattr *elem_payload = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "payload");
  struct rtattr *payload_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA); 
  addattr_uint32(n, PAYLOAD_MAX, NFTA_PAYLOAD_DREG, htonl(1));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_PAYLOAD_BASE, htonl(NFT_PAYLOAD_TRANSPORT_HEADER));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_PAYLOAD_OFFSET, htonl(2));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_PAYLOAD_LEN, htonl(2));
  addattr_nest_end(n, payload_expr_data);
  addattr_nest_end(n, elem_payload);

  struct rtattr *elem_cmp_port = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "cmp");
  struct rtattr *cmp_port_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_CMP_SREG, htonl(1));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_CMP_OP, htonl(NFT_CMP_EQ));
  struct rtattr *cmp_port_eq_value = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_CMP_DATA);
  addattr_uint16(n, PAYLOAD_MAX, NFTA_DATA_VALUE, htons(dest_port));
  addattr_nest_end(n, cmp_port_eq_value); 
  addattr_nest_end(n, cmp_port_expr_data);
  addattr_nest_end(n, elem_cmp_port);

  // * forward to */
  /* 
   * load imm (addr) => r2
   */
  struct rtattr *elem_imm_addr = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "immediate");
  struct rtattr *imm_addr_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_IMMEDIATE_DREG, htonl(2));
  struct rtattr *imm_addr_value = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_IMMEDIATE_DATA);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_DATA_VALUE, addr_i);
  addattr_nest_end(n, imm_addr_value); 
  addattr_nest_end(n, imm_addr_expr_data);
  addattr_nest_end(n, elem_imm_addr);

  // * forward to */
  /* 
   * load imm (port) => r1
   */
  struct rtattr *elem_imm_port = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "immediate");
  struct rtattr *imm_port_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_IMMEDIATE_DREG, htonl(1));
  struct rtattr *imm_port_value = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_IMMEDIATE_DATA);
  addattr_uint16(n, PAYLOAD_MAX, NFTA_DATA_VALUE, htons(target_port));
  addattr_nest_end(n, imm_port_value); 
  addattr_nest_end(n, imm_port_expr_data);
  addattr_nest_end(n, elem_imm_port);
  

  /* DNAT */
  /* dnat_ipv4 r1 */
  struct rtattr *elem_dnat = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "nat");
  struct rtattr *elem_dnat_expr_data = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_EXPR_DATA); 
  addattr_uint32(n, PAYLOAD_MAX, NFTA_NAT_TYPE, htonl(NFT_NAT_DNAT));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_NAT_FAMILY, htonl(NFPROTO_IPV4));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_NAT_REG_ADDR_MIN, htonl(2));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_NAT_REG_PROTO_MIN, htonl(1));
  addattr_nest_end(n, elem_dnat_expr_data);
  addattr_nest_end(n, elem_dnat);
  addattr_nest_end(n, rule);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;  
  

}