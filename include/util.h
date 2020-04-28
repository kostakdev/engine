#ifndef KOSTAK_UTIL_H
#define KOSTAK_UTIL_H 1

typedef struct ptr_vec {
  void **ptrs;
  size_t count;
  size_t cap;
} ptr_vec_t;

static const size_t INIT_SIZE = 256;

void init_ptr_vec(ptr_vec_t *vec);
int add_ptr(ptr_vec_t *vec, void *ptr);
void free_ptr_vec(ptr_vec_t *vec);

#endif /* KOSTAK_UTIL_H */