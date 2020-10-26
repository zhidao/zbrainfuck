/*
 * zbfc - a compiler of Brainfuck featuring gcc by Zhidao.
 *
 * 2004. 1. 3. Created.
 * 2020.10.26. Last updated.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mkstemp(char *template);

#define CC "/usr/bin/gcc"
#define CFLAGS "-Wall -O3"

void output_header(FILE *fout)
{
  fprintf( fout, "#include <stdio.h>\n" );
  fprintf( fout, "#include <stdlib.h>\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "#ifdef  FALSE\n" );
  fprintf( fout, "# undef FALSE\n" );
  fprintf( fout, "#endif\n" );
  fprintf( fout, "#define FALSE 0\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "#ifdef  TRUE\n" );
  fprintf( fout, "# undef TRUE\n" );
  fprintf( fout, "#endif\n" );
  fprintf( fout, "#define TRUE  1\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "typedef signed char val_t;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "#define MEM_BLK_SIZ 256\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "typedef struct _mem_pool_t{\n" );
  fprintf( fout, "  struct _mem_pool_t *prev, *next;\n" );
  fprintf( fout, "  val_t *mem;\n" );
  fprintf( fout, "} mem_pool_t;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "static mem_pool_t mem_pool, *cur_pool;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "void mem_pool_add(void)\n" );
  fprintf( fout, "{\n" );
  fprintf( fout, "  mem_pool_t *pool;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "  if( ( pool = calloc( 1, sizeof(mem_pool_t) ) ) == NULL )\n" );
  fprintf( fout, "    exit( 1 );\n" );
  fprintf( fout, "  if( ( pool->mem = calloc( MEM_BLK_SIZ, sizeof(val_t) ) ) == NULL )\n" );
  fprintf( fout, "    exit( 1 );\n" );
  fprintf( fout, "  if( ( pool->prev = mem_pool.prev ) )\n" );
  fprintf( fout, "    pool->prev->next = pool;\n" );
  fprintf( fout, "  pool->next = &mem_pool;\n" );
  fprintf( fout, "  mem_pool.prev = pool;\n" );
  fprintf( fout, "}\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "void mem_pool_clean(void)\n" );
  fprintf( fout, "{\n" );
  fprintf( fout, "  mem_pool_t *mp;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "  while( ( mp = mem_pool.prev ) != NULL ){\n" );
  fprintf( fout, "    mem_pool.prev = mp->prev;\n" );
  fprintf( fout, "    free( mp->mem );\n" );
  fprintf( fout, "    free( mp );\n" );
  fprintf( fout, "  }\n" );
  fprintf( fout, "}\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "void bf_init(void)\n" );
  fprintf( fout, "{\n" );
  fprintf( fout, "  mem_pool.next = mem_pool.prev = NULL;\n" );
  fprintf( fout, "  mem_pool_add();\n" );
  fprintf( fout, "}\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "void bf_end(void)\n" );
  fprintf( fout, "{\n" );
  fprintf( fout, "  mem_pool_clean();\n" );
  fprintf( fout, "}\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "#define pointer_inc(ptr) \\\n" );
  fprintf( fout, "  if( ptr >= cur_pool->mem + MEM_BLK_SIZ - 1 ){\\\n" );
  fprintf( fout, "    if( cur_pool->next == &mem_pool )\\\n" );
  fprintf( fout, "      mem_pool_add();\\\n" );
  fprintf( fout, "    ptr = ( cur_pool = cur_pool->next )->mem;\\\n" );
  fprintf( fout, "  } else\\\n" );
  fprintf( fout, "    ptr++;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "#define pointer_dec(ptr) \\\n" );
  fprintf( fout, "  if( ptr == cur_pool->mem ){\\\n" );
  fprintf( fout, "    if( ( cur_pool = cur_pool->prev ) == NULL ) exit( 1 );\\\n" );
  fprintf( fout, "    ptr = cur_pool->mem + MEM_BLK_SIZ - 1;\\\n" );
  fprintf( fout, "  } else\\\n" );
  fprintf( fout, "    ptr--;\n" );
  fprintf( fout, "\n" );
  fprintf( fout, "int main(void)\n" );
  fprintf( fout, "{\n" );
  fprintf( fout, "  register val_t *pointer;\n" );
  fprintf( fout, "  bf_init();\n" );
  fprintf( fout, "  pointer = ( cur_pool = mem_pool.prev )->mem;\n" );
}

void output_footer(FILE *fout)
{
  fprintf( fout, "  bf_end();\n" );
  fprintf( fout, "  return 0;\n" );
  fprintf( fout, "}\n" );
}

void output_body(FILE *fin, FILE *fout)
{
  register char c;

  while( !feof( fin ) )
    switch( ( c = fgetc( fin ) ) ){
    case '>':
      fprintf( fout, "pointer_inc( pointer );\n" );
      break;
    case '<':
      fprintf( fout, "pointer_dec( pointer );\n" );
      break;
    case '+':
      fprintf( fout, "(*pointer)++;\n" );
      break;
    case '-':
      fprintf( fout, "(*pointer)--;\n" );
      break;
    case '.':
      fprintf( fout, "putchar( *pointer );\n" );
      break;
    case ',':
      fprintf( fout, "*pointer = getchar();\n" );
      break;
    case '[':
      fprintf( fout, "while( *pointer ){\n" );
      break;
    case ']':
      fprintf( fout, "}\n" );
      break;
    default: ;
    }
}

FILE *open_source(char *src)
{
  FILE *fp;

  if( !src || !( fp = fopen( src, "r" ) ) )
    exit( 1 );
  return fp;
}

static char c_src[BUFSIZ];
FILE *create_tmp(char *src)
{
  FILE *fp;
  int fd;

  sprintf( c_src, "%sXXXXXX", src );
  if( ( fd = mkstemp( c_src ) ) == -1 )
    exit( 1 );
  close( fd );
  unlink( c_src );
  strcat( c_src, ".c" );
  if( !( fp = fopen( c_src, "w" ) ) )
    exit( 1 );
  return fp;
}

int compile(char *src)
{
  char cmd[BUFSIZ];

  sprintf( cmd, "%s %s %s", CC, CFLAGS, src );
  return system( cmd );
}

int main(int argc, char *argv[])
{
  FILE *fin, *fout;

  fin = open_source( argv[1] );
  fout = create_tmp( argv[1] );

  output_header( fout );
  output_body( fin, fout );
  output_footer( fout );

  fclose( fout );
  fclose( fin );

  compile( c_src );
  unlink( c_src );
  return 0;
}
