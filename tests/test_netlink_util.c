#include <stdlib.h>
#include <stdint.h>
#include <check.h>

#include "netlink_util.h"

struct ipv4addr_table {
  const char *addr_string;
  const int status;
  const uint32_t ipaddr;
  const uint32_t bitlen;
};

const struct ipv4addr_table test_ipv4addr[] = {
  {"10.2.3.1", 0, 0x0A020301, 24},
  {"10.2.3.1/16", 0, 0x0A020301, 16},
  {"",-1, /* ignored */ 0x0,24},
  {"1.2/8", 0, 0x01020000, 8},
};

START_TEST(test_parse_ipv4addr) {
  const size_t count = sizeof(test_ipv4addr) / sizeof(struct ipv4addr_table);
  int i;
  
  for(i = 0; i < count; ++i) {
    uint32_t ret;
    uint32_t bitlen;
    const struct ipv4addr_table item = test_ipv4addr[i];
    const int status = parse_addrv4(item.addr_string, &ret, &bitlen);

    ck_assert_int_eq(item.status, status);
    if (status == 0) {
      ck_assert_int_eq(htonl(item.ipaddr), ret);
      ck_assert_int_eq(item.bitlen, bitlen);
    }
  }
}
END_TEST

int main() {
  
  SRunner *sr;

  Suite *s;
  TCase *tc_core;
  
  s = suite_create("Netlink Utility");
  tc_core = tcase_create("IPv4 addr parsing");

  tcase_add_test(tc_core, test_parse_ipv4addr);
  suite_add_tcase(s, tc_core);

  int number_failed = 0;
  
  sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);

  srunner_add_suite(sr, s);
  
  srunner_run_all(sr, CK_VERBOSE);
  
  number_failed = srunner_ntests_failed(sr);
  
  // srunner_free(sr); // why this crashed
  
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}



