#ifndef CRITBIT_H_INCLUDED
#define CRITBIT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  void *root;
} strmap_t;

strmap_t *strmap_new();
void *strmap_find(strmap_t *map, char *key);
void *strmap_insert(strmap_t *map, char *key,void *data);
void *strmap_remove(strmap_t *map, char *key);
void *strmap_destroy(strmap_t *map);

#ifdef __cplusplus
};
#endif

#endif
