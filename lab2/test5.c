#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

//test 256 level

int main() {
  char *a = malloc(255);
  printf("a: %p\n", a);
  char *b = malloc(254);
  printf("b: %p\n", b);
  char *c = malloc(253);
  printf("c: %p\n", c);
  free(b);
  char *d = malloc(256);
  assert( d == b );
  printf("d: %p\n", d);
  free(c);
  char *e = malloc(200);
  printf("e: %p\n", e);
  
  assert(c==e);

  return (errno);
}