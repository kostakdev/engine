#ifndef NET_FORWARD
#define NET_FORWARD  1

typedef enum nat_hook_chain{
  NAT_PREROUTING = NF_INET_PRE_ROUTING,
  NAT_POSTROUTING = NF_INET_POST_ROUTING,
  NAT_OUTPUT = NF_INET_LOCAL_OUT,
} nat_hook_chain_t;

int start_nf_batch(void *buf, void **nextbuf);

int end_nf_batch(void *buf, void **nextbuf);

int flush_rules(void *buf, void **nextbuf);

int create_table(const char *table_name, void *buf, void **nextbuf);

int create_nat_chain(const char *table_name, const char *chain_name,
                     const nat_hook_chain_t hook_type,
                     void *buf, void **nextbuf);

int create_masq_rule(const char *table_name, const char *chain_name, 
                        void *buf, void **nextbuf);

int create_tcp_portforward_rule(const char *table_name, const char *chain_name,
  const __u16 dest_port, const char *dest_addr, void *buf, void **nextbuf);


#endif /* NET_FORWARD */