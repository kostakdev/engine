#ifndef COMPAT_H
#define COMPAT_H 1

#ifndef htonl
uint32_t htonl(uint32_t hostlong);
#endif

#ifndef htons
uint16_t htons(uint16_t hostshort);
#endif

#ifndef ntohl
uint32_t ntohl(uint32_t netlong);
#endif

#ifndef ntohs
uint16_t ntohs(uint16_t netshort);
#endif

#endif /* COMPAT_H */

