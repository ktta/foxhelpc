#ifndef _djgkjdioweoif_buffer_h_included
#define _djgkjdioweoif_buffer_h_included

typedef struct buffer buffer_t;

#ifndef _BUFFER_PROTO
#define _BUFFER_PROTO 
#endif

_BUFFER_PROTO buffer_t *buffer_new(size_t blksize);
_BUFFER_PROTO void buffer_free(buffer_t *B, void **Rd, size_t *Rl);
_BUFFER_PROTO void buffer_collect(buffer_t *B, void **Rd, size_t *Rl);
_BUFFER_PROTO void buffer_write(buffer_t *B, void *d, size_t l);

#endif
