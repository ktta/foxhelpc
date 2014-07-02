#include <stdio.h> 
#include <stdint.h> 
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <errno.h> 
#include "critbit.h"

typedef struct
{
  void* child[2];
  unsigned internal:1, byte:23, mask:8;
} strmap_node_t;


static inline int strmap_dir
    (uint8_t *key,uint32_t len_key,strmap_node_t *node)
{
  uint8_t c;

  if (node->byte<len_key) c= key[node->byte];
                     else c= 0;
  return (1+(node->mask|c))>>8;
}

void* strmap_find(strmap_t *map,char *skey)
{
  uint8_t *key; unsigned int len_key;
  strmap_node_t *node;

  node= map->root;
  if (!node) return 0;

  key= (void*) skey;
  len_key= strlen(skey);

  while(node->internal)
     node= node->child[strmap_dir(key,len_key,node)];

  if (strcmp(skey, node->child[0])) return 0;
                               else return node->child[1];
}


static inline uint32_t strmap_make_mask
    (uint32_t A, uint32_t B)
{
  uint8_t diff;
  diff= A ^ B;
  diff|= diff>>1;
  diff|= diff>>2;
  diff|= diff>>4;
  return (diff&~(diff>>1))^255;
}

static inline strmap_node_t* strmap_leaf_node
    (uint8_t *key, void *data)
{
  strmap_node_t *N;
  N= malloc(sizeof(*N));
  N->internal= 0;
  N->child[0]= key;
  N->child[1]= data;
  return N;
}

static inline strmap_node_t* strmap_internal_node
    (uint32_t byte, uint32_t mask)
{
  strmap_node_t *N;
  N= malloc(sizeof(*N));
  N->internal= 1;
  N->byte= byte;
  N->mask= mask;
  return N;
}

void* strmap_insert(strmap_t* map, char* skey, void *data)
{
  uint8_t *key; uint32_t len_key;
  strmap_node_t *node_elt;

  key= (void*) skey;

  if (!map->root)
  {
     map->root= strmap_leaf_node(key,data);
     return 0;
  }

  len_key= strlen(skey);

  node_elt= map->root;
  while(node_elt->internal)
    node_elt= node_elt->child[strmap_dir(key,len_key,node_elt)];

  uint32_t crit_byte;
  uint8_t  mask;
  uint8_t  *elt;
  int dir_elt;
  strmap_node_t *node_key;

  elt= node_elt->child[0];

  for(crit_byte=0;
      crit_byte<=len_key && elt[crit_byte]==key[crit_byte];
      crit_byte++)
          ;

  if (crit_byte>len_key) return node_elt->child[1];

  mask= strmap_make_mask(elt[crit_byte],key[crit_byte]);

  dir_elt= (1+(mask|elt[crit_byte]))>>8;

  node_key= strmap_internal_node(crit_byte,mask);
  node_key->child[1-dir_elt]= strmap_leaf_node(key,data);

  strmap_node_t *parent;
  strmap_node_t *node;

  parent= 0;
  node= map->root;

  while(node->internal)
  {
     if (node->byte > crit_byte) break;
     if (node->byte==crit_byte && node->mask > mask) break;
     parent= node;
     node= node->child[strmap_dir(key,len_key,node)];
  }

  node_key->child[dir_elt]= node;
  if (parent) { if (parent->child[0]==node) parent->child[0]= node_key;
                                    else    parent->child[1]= node_key; }
  else map->root= node_key;
  return 0;
}

void *strmap_remove(strmap_t* map, char *skey)
{
  uint8_t *key; uint32_t len_key;
  void *retv;

  if (!map->root) return 0;

  key= (void*) skey;
  len_key= strlen(skey);
   
  strmap_node_t *node;
  strmap_node_t *sibling;
  strmap_node_t *parent;
  strmap_node_t *gparent;

  node= map->root;
  gparent= 0;
  sibling= 0;
  parent= 0;

  while(node->internal)
  {
    int dir;
    dir= strmap_dir(key, len_key, node);
    gparent= parent;
    parent= node;
    sibling= node->child[1-dir];
    node= node->child[dir];
  }

  if (strcmp(node->child[0],skey)) return 0;
  retv= node->child[1];
  free(node);

  if (!gparent)
  {
     map->root= sibling;
     if (parent) free(parent);
     return retv;
  }

  if (gparent->child[0]==parent)
    gparent->child[0]= sibling;
  else
    gparent->child[1]= sibling;

  free(parent);

  return retv;
}

void *strmap_destroy(strmap_t *map)
{
  strmap_node_t *node;
  strmap_node_t *parent;
  strmap_node_t *gparent;
  strmap_node_t *sibling;
  void *retv;

  if (!map->root) return 0;

  parent= 0;
  gparent= 0;
  sibling= 0;
  node= map->root;
  while(node->internal)
  {
    gparent= parent;
    parent= node;
    sibling= node->child[1];
    node= node->child[0];
  }
  retv= node->child[1];
  free(node);
  if (!gparent) 
  {
    map->root= sibling;
    if (parent) free(parent);
    return retv;
  }
  if (gparent->child[0]==parent)
    gparent->child[0]= sibling;
  else
    gparent->child[1]= sibling;

  free(parent);

  return retv;
}

strmap_t  *strmap_new()
{
  strmap_t *s;
  s= malloc(sizeof(*s));
  s->root= 0;
  return s;
}
