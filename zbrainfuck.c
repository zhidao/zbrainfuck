/*
 * zbrainfuck - an interpreter of Brainfuck by Zhidao.
 *
 * 2004. 1. 3. Created.
 * 2004. 1. 4. Last updated.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef  FALSE
# undef FALSE
#endif
#define FALSE 0

#ifdef  TRUE
# undef TRUE
#endif
#define TRUE  1

typedef signed char val_t;

/* memory pool list */

#define MEM_BLK_SIZ 256

typedef struct _mem_pool_t{
  struct _mem_pool_t *prev, *next;
  val_t *mem;
} mem_pool_t;

static mem_pool_t mem_pool, *cur_pool;

void mem_pool_add(void)
{
  mem_pool_t *pool;

  if( ( pool = calloc( 1, sizeof(mem_pool_t) ) ) == NULL )
    exit( 1 );
  if( ( pool->mem = calloc( MEM_BLK_SIZ, sizeof(val_t) ) ) == NULL )
    exit( 1 );
  if( ( pool->prev = mem_pool.prev ) )
    pool->prev->next = pool;
  pool->next = &mem_pool;
  mem_pool.prev = pool;
}

void mem_pool_clean(void)
{
  mem_pool_t *mp;

  while( ( mp = mem_pool.prev ) != NULL ){
    mem_pool.prev = mp->prev;
    free( mp->mem );
    free( mp );
  }
}

/* source buffer */

static val_t *buf;
static unsigned long buf_size;

void read_source(char *src)
{
  FILE *fp;
  register unsigned long cur;

  if( !src || !( fp = fopen( src, "r" ) ) )
    exit( 1 );
  fseek( fp, 0, SEEK_END );
  if( !( buf = calloc( ( buf_size = ftell( fp ) ), sizeof(val_t) ) ) )
    exit( 1 );
  rewind( fp );
  for( cur=0; cur<buf_size; cur++ )
    buf[cur] = fgetc( fp );
  fclose( fp );
}

/* loop stack */

typedef struct _loop_stack_t{
  unsigned long cur;
  unsigned char flag;
  struct _loop_stack_t *next;
} loop_stack_t;
static loop_stack_t loop;

static unsigned char flag = TRUE;

void loop_push(unsigned long cur)
{
  loop_stack_t *stack;

  if( ( stack = malloc( sizeof(loop_stack_t) ) ) == NULL )
    exit( 1 );
  stack->cur = cur - 1;
  stack->flag = flag;
  stack->next = loop.next;
  loop.next = stack;
}

unsigned long loop_pop(unsigned long cur)
{
  loop_stack_t *stack;

  if( ( stack = loop.next ) == NULL ) exit( 1 );
  loop.next = stack->next;
  if( flag ) cur = stack->cur;
  flag = stack->flag;
  free( stack );
  return cur;
}

void loop_clean(void)
{
  loop_stack_t *stack;

  while( ( stack = loop.next ) != NULL ){
    loop.next = stack->next;
    free( stack );
  }
}

/* intialization and termination */

void bf_init(void)
{
  mem_pool.next = mem_pool.prev = NULL;
  mem_pool_add();
  loop.next = NULL;
}

void bf_end(void)
{
  loop_clean();
  mem_pool_clean();
  free( buf );
}

/* main loop */

void interpret(void)
{
  register val_t *pointer;
  register unsigned long cur;

  pointer = ( cur_pool = mem_pool.prev )->mem;
  for( cur=0; cur<buf_size; cur++ )
    switch( buf[cur] ){
    case '>':
      if( !flag ) break;
      if( pointer >= cur_pool->mem + MEM_BLK_SIZ - 1 ){
        if( cur_pool->next == &mem_pool )
          mem_pool_add();
        pointer = ( cur_pool = cur_pool->next )->mem;
      } else
        pointer++;
      break;
    case '<':
      if( !flag ) break;
      if( pointer == cur_pool->mem ){
        if( ( cur_pool = cur_pool->prev ) == NULL ) exit( 1 );
        pointer = cur_pool->mem + MEM_BLK_SIZ - 1;
      } else
        pointer--;
      break;
    case '+':
      flag ? (*pointer)++ : 0;
      break;
    case '-':
      flag ? (*pointer)-- : 0;
      break;
    case '.':
      flag ? putchar( *pointer ) : 0;
      break;
    case ',':
      flag ? ( *pointer = getchar() ) : 0;
      break;
    case '[':
      loop_push( cur );
      flag = *pointer ? TRUE : FALSE;
      break;
    case ']':
      cur = loop_pop( cur );
      break;
    default: ;
    }
}

int main(int argc, char *argv[])
{
  read_source( argv[1] );
  bf_init();
  interpret();
  bf_end();
  return 0;
}
