#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "critbit.c"




void dump_node(strmap_node_t *node)
{
  if (node->internal) {
    printf("[%d,%1x,", node->byte, node->mask);
    dump_node(node->child[0]);
    printf(",");
    dump_node(node->child[1]);
    printf("]");
 } else {
    printf("(%s)", node->child[0]);
 }
    
}

void dump_map(strmap_t *map)
{
  if (map->root) dump_node(map->root);
  else printf("[EMPTY]");
}

#define JJ 20

typedef struct {
  char str[JJ+1];
  int value;
} aa_t;

#define N 10000
#define M 20000

strmap_t *s;
aa_t bb[N];
aa_t *in[N];
aa_t *out[N];
int n_in;
int n_out;

int n_insert,n_delete,n_lookup;

void fail()
{
  printf("%d insertions, %d lookups, %d deletions\n",
       n_insert, n_lookup, n_delete);
  dump_map(s);
  printf("\n");
  fflush(0);
  *(int*)0= 0;
}

void prepare(int k)
{
  int i;
  int L;
  bb[k].value= k;
again:
  L= rand() % JJ+1;
  for(i=0;i<L;i++)
     bb[k].str[i]= rand()%26+'a';
  bb[k].str[i]= 0;

  for(i=0;i<k;i++) if (!strcmp(bb[k].str,bb[i].str)) goto again;
  out[k]= bb+k;
  n_out++;
}


void do_find()
{
  if (!n_in) return ;
  n_lookup++;
  int K= rand()%n_in;

  printf("L> %s\n", in[K]->str);

  aa_t *R= strmap_find(s,in[K]->str);
  if (R!=in[K])
    { printf("string %s in table but not found. \n"
             "   returned ptr= %08x\n", in[K]->str, (unsigned) R ); fail(); }

  dump_map(s); printf("\n");
}


void do_remove()
{
  if (!n_in) return ;
  n_delete++;
  int K= rand()%n_in;

  printf("R> %s\n", in[K]->str);

  aa_t *R= strmap_remove(s,in[K]->str);
  if (R!=in[K]) 
    { printf("string %s in table but not found.\n", in[K]->str); fail(); }


  aa_t *Z;
  Z= in[K];
  in[K]= in[n_in-1];
  out[n_out]= Z;
  n_in--; n_out++;

  R= strmap_find(s, Z->str);
  if (R)
  {
    printf("string %s was removed, but still can be found.\n",
           Z->str); fail(); 
  }
  dump_map(s); printf("\n");
}

void do_insert()
{
  if (!n_out) return ;
  n_insert++;
  int K= rand()%n_out;

  printf("I> %s\n", out[K]->str);

  aa_t *R= strmap_insert(s,out[K]->str, out[K]);
  if (R)
    { printf("tried to insert string %s but it's already there.\n",
      out[K]->str);
      printf("returned pointer= %08x (%s,%d)\n"
              "tried pointer=    %08x (%s,%d)\n",
          (unsigned) R, R->str, R->value,
          (unsigned) out[K], out[K]->str, out[K]->value);
      fail(); }


  aa_t *Z;
  Z= out[K];
  out[K]= out[n_out-1];
  in[n_in]= Z;
  n_in++; n_out--;


  R= strmap_find(s, Z->str);
  if (!R)
  {
     printf("string %s was just inserted, but it can't be found.\n",
        Z->str);
     fail();
  }
  dump_map(s); printf("\n");
}

int main()
{
  int i;
  s= strmap_new();
  srand(time(0));
  for(i=0;i<N;i++)
    prepare(i);
  // do_insert(); do_insert(); do_remove(); do_find(); return 0;
  for(i=0;i<M;i++)
  {
    int op= rand()%100;
    if (op<30) do_find();
    else if (op<60) do_insert();
    else do_remove();
  }
  aa_t *Z;
  while((Z=strmap_destroy(s))) {
    printf("<D %s\n", Z->str);
    dump_map(s);
    printf("\n");
  }
  free(s);
  printf("%d insertions, %d lookups, %d deletions\n",
       n_insert, n_lookup, n_delete);
  return 0;
}



