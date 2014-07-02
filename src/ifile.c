#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ifile.h"

#define IFILE_BLOCK_SIZE 128

typedef struct ifileblock {
  uint8_t data[IFILE_BLOCK_SIZE];
  size_t len;
  struct ifileblock *next;
} ifileblock_t;

struct ifile { 
  FILE *handle;
  ifileblock_t *blocks,*last;
  int eof, lineno;
  char *name, *error;
};

int ifile_open(ifile_t **Rf,char *name)
{
  ifile_t *f;
  char buf[2000];
  int R;
  R= 0;

  f= malloc(sizeof(*f));
  f->handle= fopen(name, "r");
  if (f->handle) 
  {
     f->error= NULL;
     f->eof= 0;
  } else {
     snprintf(buf, sizeof(buf), "can't open %s: %s",name, strerror(errno));
     f->error= strdup(buf);
     R= 1;
     f->eof= 1;
  }
  f->lineno= 0;
  f->name= strdup(name);
  f->blocks= f->last= NULL;
  *Rf= f;
  return R;
}

static int ifile_read_block(ifile_t *f)
{
  ifileblock_t *B;
  size_t R;

  if (f->eof) return 1;

  B= malloc(sizeof(*B));
  R= fread(B->data, 1, sizeof(B->data), f->handle);
  if (R<sizeof(B->data))
  {
     if (R==0) free(B);
     f->eof= 1;
     if (ferror(f->handle))
     {
       char buf[2000];
       snprintf(buf,sizeof(buf), "read error from %s: %s", 
                          f->name, strerror(errno));
       f->error= strdup(buf);
     }
     if (R==0) return 1;
  }

  B->len= R;
  B->next= NULL;
  if (!f->blocks) { f->blocks= f->last= B; }
  else { f->last->next= B; f->last= B; }
  return 0;
}


static void ifile_collect_blocks
    (ifile_t *f, ifileblock_t *E,size_t endi,
     uint8_t **Rbuffer, size_t *Rlen, int nullterm)
{
   ifileblock_t *B,*N;
   uint8_t *buffer;

   size_t size;
   size= 0;
   if (endi==E->len-1) { E=NULL; endi= -1; }
   for(B=f->blocks;B!=E;B=B->next) size+= B->len;
   size+= endi+1; 

   *Rlen= size;

   buffer= malloc(size + nullterm);
   size= 0;
   for(B=f->blocks;B!=E;B=N)
   {
     N= B->next;
     memcpy(buffer+size, B->data, B->len);
     size+= B->len;
     free(B);
   }

   if (E) {
     memcpy(buffer+size, E->data, endi+1);
     memmove(E->data, E->data+endi+1, E->len-endi-1);
     E->len-= endi+1;
     size+= endi+1;
   }

   f->blocks= E;
   *Rbuffer= buffer;
   if (nullterm) { int i; for(i=0;i<nullterm;i++) buffer[size+i]= 0; }
}


int ifile_get_line(ifile_t *f,uint8_t **buffer, size_t *len,int nullterm)
{
  ifileblock_t *L;
  int i;

  if (!f->blocks && ifile_read_block(f)) return 1;

again:
  i= 0;
  L= f->last;
next:
  if (i==L->len)
  {
    if (ifile_read_block(f)) goto done;
    goto again;
  }
  if (L->data[i]=='\n') goto done;

  if (L->data[i]=='\r')
  {
     if (i==L->len-1) {
        if (ifile_read_block(f)) goto done;
        if (L->next->data[0]=='\n') { i= 0; L= L->next; }
     } else {
        if (L->data[i+1]=='\n') i++;
     }
     goto done;
  }
  i++;
  goto next;
done:
  ifile_collect_blocks(f, L,i, buffer, len,nullterm);
  f->lineno++;
  return 0;
}

void ifile_close(ifile_t *f)
{
   ifileblock_t *B,*N;
   if (f->handle) fclose(f->handle);
   for(B=f->blocks;B;B=N) { N= B->next; free(B); }
   if (f->error) free(f->error);
   free(f->name);
   free(f);
}

int ifile_line_number(ifile_t *f) { return f->lineno; }
char *ifile_error(ifile_t *f) { return f->error; }
char *ifile_file_name(ifile_t *f) { return f->name; }


