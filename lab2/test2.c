#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

//test 128 level

int main() {
  char *a = malloc(128);
  printf("a: %p\n", a);
  char *b = malloc(128);
  printf("b: %p\n", b);
  char *c = malloc(128);
  printf("c: %p\n", c);
  free(b);
  char *d = malloc(128);
  printf("d: %p\n", d);
  free(c);
  char *e = malloc(128);
  printf("e: %p\n", e);
  char *f = malloc(128);
  printf("f: %p\n", f);

  return (errno);
}
