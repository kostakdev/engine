#ifndef KOSTAK_UTIL_H
#define KOSTAK_UTIL_H 1

typedef struct ptr_vec {
  void **ptrs;    /* Array of pointers */
  size_t count;   /* Total count of the pointers */
  size_t cap;     /* Total capacity of the array of pointers */
} ptr_vec_t;

static const size_t INIT_SIZE = 256;

void init_ptr_vec(ptr_vec_t *vec, void **init_values, const size_t count);
int add_ptr(ptr_vec_t *vec, void *ptr);
void free_ptr_vec(ptr_vec_t *vec);

#endif /* KOSTAK_UTIL_H */
