#include <stdlib.h>
#include <string.h>
#include "buffer.h"


struct buffer_node { 
  struct buffer_node *next;
  size_t len;
  unsigned char data[1];
};

struct buffer {
  struct buffer_node *first, *last;
  size_t blksize;
};


_BUFFER_PROTO buffer_t *buffer_new(size_t blksize)
{
  buffer_t *B;
  B= malloc(sizeof(*B));
  B->first= B->last= NULL;
  B->blksize= blksize;
  return B;
}


_BUFFER_PROTO void buffer_free(buffer_t *B, void **Rd, size_t *Rl)
{
  struct buffer_node *P,*N;
  if (Rd || Rl) buffer_collect(B, Rd, Rl);
  for(P=B->first;P;P=N)
  {
    N= P->next;
    free(P);
  }
  free(B);
}

_BUFFER_PROTO void buffer_collect(buffer_t *B, void **Rd, size_t *Rl)
{
  size_t S;
  struct buffer_node *P;
  unsigned char *R;

  if (!Rd && !Rl) return ;

  S= 0;
  for(P=B->first;P;P=P->next) S+= P->len;

  if (S==0) { if (Rd) *Rd= NULL; if (Rl) *Rl= 0; return ; }
  if (!Rd) { *Rl= S; return ; }

  R= malloc(S);
  S= 0;
  for(P=B->first;P;P=P->next)
  {
    memcpy(R+S, P->data, P->len);
    S+= P->len;
  }

  *Rd= (void*) R;
  if (Rl) *Rl= S;
}

_BUFFER_PROTO void buffer_write(buffer_t *B, void *d, size_t l)
{
  struct buffer_node *P;
  size_t cp, spc;

  while(l)
  {
    if (!B->last || B->last->len==B->blksize)
    {
      P= malloc(sizeof(*P)+B->blksize-1);
      P->len= 0;
      P->next= NULL;
      if (!B->last) { B->last= B->first= P; }
      else { B->last->next= P; B->last= P; }
    }

    spc= B->blksize - B->last->len;
    if (spc>l) cp= l; else cp= spc;

    memcpy(B->last->data+B->last->len, d, cp);
    B->last->len+= cp;
    l-= cp;
    d= (unsigned char*) d + cp;
  }
}

