#ifndef KOSTAK_UTIL_H
#define KOSTAK_UTIL_H 1

/* 
 * Simple string tuple implementation
 * It's just an array of pascalstring
 */

typedef struct strtuple {
  char *buf;
  char **bufarr;
} strtuple_t;

#endif /* KOSTAK_UTIL_H */